let elementProperties = {}
let systemDetails = {
	"audio_level": 0,
	"audio_status": 0,
	"net_status": 0,
	"net_signal_strength": 0,
	"brightness": 0,
	"disk_usage": 0,
	"cpu_usage": 0,
	"ram_usage": 0,
	"workspace": "Workspace { id: 1, monitor_id: 0, focused: true }",
	"workspaces": "",
	"client": "Client { id: 0, initial_title: \"\", title: \"\", class: \"\", instance: \"\", pid: 0 }",
	"bat_capacity": 100,
	"bat_status": 1
}

let containers = {}
let slots = {}

let clock;
let activeWindow;
let battery;

let newData = {};

const ws = new WebSocket("ws://localhost:3001");
ws.onopen = () => {
	ws.send(JSON.stringify({ type: "request_update" }));
}

ws.onmessage = (event) => {
	newData = JSON.parse(event.data);
	updateAll();
}

// Listen for 'config' events to set up the status bar elements
window.addEventListener('config', (e) => {
    elementProperties = e.detail || window.AetherBarConfig || {};

    containers.left = document.getElementById('left');
    containers.center = document.getElementById('center');
    containers.right = document.getElementById('right');
    
    for (const pos of ['left', 'center', 'right']) {
        const el = containers[pos];
        if (el) Array.from(el.querySelectorAll('.slot, .spacer')).forEach(n => n.remove());
    }
    
    slots = {};
    const appendedCount = { left: 0, center: 0, right: 0 };
    for (const [name, cfg] of Object.entries(elementProperties)) {
        if (!cfg || typeof cfg !== 'object') continue;
        const pos = String(cfg.position || 'right').toLowerCase();
        const posKey = (pos === 'left' || pos === 'center' || pos === 'right') ? pos : 'right';
        const parent = containers[posKey] || containers.right;
        if (!parent) continue;
        const node = document.createElement('div');
        node.id = `slot-${name}`;
        node.className = `slot slot-${name}`;
        if (appendedCount[posKey] > 0) {
            const spacer = document.createElement('div');
            spacer.className = 'spacer';
            parent.appendChild(spacer);
        }
        parent.appendChild(node);
        appendedCount[posKey]++;
        slots[name] = node;
    }
});

const updateAll = () => {
	const detail = newData.datapack || {};
    systemDetails = { ...systemDetails, ...detail };

    updateClock();
    if ('client' in detail) updateActiveClient();
    if ('bat_capacity' in detail || 'bat_status' in detail) updateBattery();
    if ('workspace' in detail || 'workspaces' in detail) updateWorkspace();
    if ('cpu_usage' in detail) updateCPU();
    if ('ram_usage' in detail) updateMemory();
    if ('disk_usage' in detail) updateDisk();
    if ('brightness' in detail) updateBrightness();
    if ('net_status' in detail || 'net_signal_strength' in detail) updateNetwork();
    if ('audio_level' in detail || 'audio_status' in detail) updateAudio();

	setTimeout(() => {
		ws.send(JSON.stringify({ type: "request_update" }));
	}, 100);
}

// Function to update the audio volume display
const updateAudio = () => {
    const el = slots.volume;
    if (!el) return;

	let vol = systemDetails.audio_level;

    let icon = '';
    if (vol === 0 || systemDetails.audio_status === 0) {
        icon = 'volume_off';
        vol = '';
    }
    else {
        if (vol <= 33) icon = 'volume_mute';
        else if (vol <= 66) icon = 'volume_down';
        else icon = 'volume_up';
        vol = ` ${vol}%`;
    }

    const iconHtml = `<img src="svg/${icon}.svg" class="volumeIcon icon">`;
    el.innerHTML = "<div class='volume'>" + iconHtml + vol + "</div>";
};

// Function to update the network status display
const updateNetwork = () => {
    const el = slots.network;
    if (!el) return;

    let icon = '';
    if (systemDetails.net_status === -1) {
        icon = 'signal_wifi_off';
    }
    else {
        if (systemDetails.net_status == 1) {
            if (systemDetails.net_signal_strength < 10) icon = 'signal_wifi_0_bar'
            else if (systemDetails.net_signal_strength < 25) icon = 'network_wifi_1_bar'
            else if (systemDetails.net_signal_strength < 50) icon = 'network_wifi_2_bar'
            else if (systemDetails.net_signal_strength < 75) icon = 'network_wifi_3_bar'
            else if (systemDetails.net_signal_strength < 90) icon = 'network_wifi'
            else icon = 'signal_wifi_4_bar'
        }
        else icon = 'lan'
    }

    let internetIcon = '';
    if (icon == 'lan') internetIcon = 'lan'
    else internetIcon = 'wifi'

    const iconHtml = `<img src="svg/${icon}.svg" class="networkIcon ${internetIcon} icon">`;
    el.innerHTML = "<div class='network'>" + iconHtml + systemDetails.net_signal_strength + "%</div>";
};

// Function to update the brightness display
const updateBrightness = () => {
    const el = slots.brightness;
    if (!el) return;

    const lvl = parseInt(String(systemDetails.brightness).replace('%', ''), 10) || 0;

    let iconName = '';
    if (lvl < 33) iconName = 'brightness_low';
    else if (lvl < 66) iconName = 'brightness_medium';
    else iconName = 'brightness_high';

    const iconHtml = `<img src="svg/${iconName}.svg" class="brightnessIcon icon">`;

    el.innerHTML = "<div class='brightness'>" + iconHtml + " " + systemDetails.brightness + "%</div>";
}

// Function to update the disk usage display
const updateDisk = () => {
    const el = slots.disk;
    if (!el) return;

    el.innerHTML = "<div class='disk'>" + ' <img src="svg/disk.svg" class="diskIcon icon">' + `<a>${systemDetails.disk_usage}%</a>` + " </div>";
}

// Function to update the CPU usage display
const updateCPU = () => {
    const el = slots.cpu;
    if (!el) return;

    el.innerHTML = "<div class='cpu'>" + ' <img src="svg/cpu.svg" class="cpuIcon icon">' + `<a>${systemDetails.cpu_usage}%</a>` + " </div>";
}

// Function to update the Memory usage display
const updateMemory = () => {
    const el = slots.memory;
    if (!el) return;

    el.innerHTML = "<div class='memory'>" + ' <img src="svg/memory.svg" class="memoryIcon icon">' + `<a>${systemDetails.ram_usage}%</a>` + " </div>";
}

// Function to update the workspace display
const updateWorkspace = () => {
    const el = slots.workspace;
    if (!el) return;

    const active = Number(systemDetails.workspace.split(",")[0].replace("Workspace { id: ", ""));

    const all = systemDetails.workspaces.split("monitor_id").length - 1;

    let allStr = "";
    for (let i = 1; i <= all; i++) {
        if (i === active) {
            allStr += `<span class="active material-symbols-outlined">mode_standby</span> `;
        } else {
            allStr += `<span class="notActive material-symbols-outlined">fiber_manual_record</span> `;
        }
    }
    
    el.innerHTML = "<div class='workspaces'>" + allStr + "</div>";
}

// Function to update the clock display
const updateClock = () => {
    const el = slots.clock;
    if (!el) return;

    const tpl = elementProperties?.clock?.format || '{HH:mm:ss}';
    const match = tpl.match(/\{([^}]*)\}/);
    const [prefix, suffix] = tpl.split(/\{[^}]*\}/);
    const pattern = match ? match[1] : 'HH:mm:ss';

    const [hStr, mStr, sStr] = new Date().toLocaleTimeString('en-US', { hour12: false }).split(':');
    const h = Number(hStr) || 0;
    const m = Number(mStr) || 0;
    const s = Number(sStr) || 0;

    const HH = String(h).padStart(2, '0');
    const hh = String((h % 12) || 12).padStart(2, '0');
    const mm = String(m).padStart(2, '0');
    const ss = String(s).padStart(2, '0');
    const a = h < 12 ? 'am' : 'pm';
    const A = a.toUpperCase();

    const uses12h = pattern.includes('hh');
    const hasMeridiem = pattern.includes('a') || pattern.includes('A');

    let formatted = pattern
        .replace(/HH/g, HH)
        .replace(/hh/g, hh)
        .replace(/mm/g, mm)
        .replace(/ss/g, ss)
        .replace(/A/g, A)
        .replace(/a/g, a);

    if (uses12h && !hasMeridiem) {
        formatted += ` ${A}`;
    }

    el.innerHTML = "<div class='clock'><img src='svg/clock.svg' class='clockIcon icon'>" + `${prefix || ''}${formatted}${suffix || ''}` + "</div>";
}

// Function to update the active client display
const updateActiveClient = () => {
    const el = slots.activeWindow;
    if (!el) return;

    let temp = String(systemDetails.client).split("class")[1];
    let temp2 = String(systemDetails.client).replace("initial_title", "initial").split("title")[1];

    el.innerHTML = "<div class='activeWindow'>" + temp2.split("pid")[0].replaceAll('",', '').replace(': "', "").trim() + " | " + temp.replaceAll('",', '').replace(': "', "").trim() + "</div>";
}

const updateBattery = () => {
    const el = slots.battery;
    if (!el) return;

    let iconName = '';
    let stateClass = '';
    switch (systemDetails.bat_status) {
        case 1:
            iconName = 'battery_android_bolt';

            stateClass = 'charging';
            break;
        default:
            if (systemDetails.bat_capacity < 13) iconName = 'battery_android_0';
            else if (systemDetails.bat_capacity  < 26) iconName = 'battery_android_1';
            else if (systemDetails.bat_capacity  < 38) iconName = 'battery_android_2';
            else if (systemDetails.bat_capacity  < 50) iconName = 'battery_android_3';
            else if (systemDetails.bat_capacity  < 63) iconName = 'battery_android_4';
            else if (systemDetails.bat_capacity  < 75) iconName = 'battery_android_5';
            else if (systemDetails.bat_capacity  < 88) iconName = 'battery_android_6';
            else iconName = 'battery_android_full';

            stateClass = 'discharging';
            break;
    }

    const iconHtml = `<img src="svg/${iconName}.svg" class="batteryIcon icon ${stateClass}">`;

    el.innerHTML = "<div class='battery'>" + iconHtml + " " + systemDetails.bat_capacity + "%</div>";
}
