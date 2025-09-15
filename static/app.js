let elementProperties = {}
let systemDetails = {}

let containers = {}
let slots = {}

let clock;
let activeWindow;
let battery;

window.addEventListener('config', (e) => {
    elementProperties = e.detail || window.AetherConfig || {};

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

window.addEventListener('tick', (e) => {
    const detail = e.detail || {};
    systemDetails = { ...systemDetails, ...detail };

    if ('time' in detail) updateClock();
    if ('client' in detail) updateActiveClient();
    if ('battery' in detail || 'battery_state' in detail) updateBattery();
    if ('workspace' in detail || 'workspaces' in detail) updateWorkspace();
    if ('cpu_usage' in detail) updateCPU();
    if ('memory_usage' in detail) updateMemory();
    if ('disk_usage' in detail) updateDisk();
    if ('brightness' in detail) updateBrightness();
});

const updateBrightness = () => {
    const el = slots.brightness;
    if (!el) return;

    const lvl = parseInt(String(systemDetails.brightness).replace('%', ''), 10) || 0;

    let iconName = 'wb_sunny';
    if (lvl < 20) iconName = 'brightness_low';
    else if (lvl < 40) iconName = 'brightness_5';
    else if (lvl < 60) iconName = 'brightness_medium';
    else if (lvl < 80) iconName = 'brightness_high';
    else iconName = 'brightness_7';

    const iconHtml = `<span class="material-icons-round">` + iconName + `</span>`;

    el.innerHTML = "<div class='brightness'>" + iconHtml + " " + systemDetails.brightness + "</div>";
}

const updateDisk = () => {
    const el = slots.disk;
    if (!el) return;

    el.innerHTML = "<div class='disk'>" + systemDetails.disk_usage + "  </div>";
}

const updateCPU = () => {
    const el = slots.cpu;
    if (!el) return;

    el.innerHTML = "<div class='cpu'>" + systemDetails.cpu_usage + "  </div>";
}

const updateMemory = () => {
    const el = slots.memory;
    if (!el) return;
    
    el.innerHTML = "<div class='memory'>" + systemDetails.memory_usage + "  </div>";
}

const updateWorkspace = () => {
    const el = slots.workspace;
    if (!el) return;

    const active = Number(systemDetails.workspace.split(",")[0].replace("Workspace { id: ", ""));

    const all = systemDetails.workspaces.split("monitor_id").length - 1;

    let allStr = "";
    for (let i = 1; i <= all; i++) {
        if (i === active) {
            allStr += `<span class="active"> </span> `;
        } else {
            allStr += `<span class="notActive"> </span> `;
        }
    }
    
    el.innerHTML = "<div class='workspaces'>" + allStr + "</div>";
}

const updateClock = () => {
    const el = slots.clock;
    if (!el) return;

    const tpl = elementProperties?.clock?.format || '{HH:mm:ss}';
    const match = tpl.match(/\{([^}]*)\}/);
    const [prefix, suffix] = tpl.split(/\{[^}]*\}/);
    const pattern = match ? match[1] : 'HH:mm:ss';

    const [hStr, mStr, sStr] = (systemDetails?.time || '00:00:00').split(':');
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

    el.innerHTML = "<div class='clock'>" + `${prefix || ''}${formatted}${suffix || ''}` + "</div>";
}

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

    const level = Number.parseInt(String(systemDetails.battery), 10);

    let iconName = '';
    switch (systemDetails.battery_state) {
        case "Charging":
            if (level <= 60) iconName = 'battery_android_bolt'
            else iconName = 'battery_android_frame_bolt'

            break;
        default:
            if (level <= 10) iconName = 'battery_android_alert'
            else if (level <= 25) iconName = 'battery_android_frame_3'
            else if (level <= 50) iconName = 'battery_android_frame_4'
            else if (level <= 75) iconName = 'battery_android_frame_5'
            else if (level <= 90) iconName = 'battery_android_frame_6'
            else iconName = 'battery_android_frame_full'
            break;
    }

    const iconHtml = `<span class="material-symbols-outlined">` + iconName + `</span>`;

    el.innerHTML = "<div class='battery'>" + iconHtml + " "+ systemDetails.battery + "</div>";
}