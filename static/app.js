let elementProperties = {}
let systemDetails = {}

let clock;
let activeWindow;
let battery;

window.addEventListener('config', (e) => {
    elementProperties = e.detail || window.AetherConfig || {};

    clock = document.getElementById(elementProperties.clock.position);
    activeWindow = document.getElementById(elementProperties.activeWindow.position);
    battery = document.getElementById(elementProperties.battery.position);
});

window.addEventListener('tick', (e) => {
    systemDetails = e.detail || {};

    updateClock();
    updateActiveClient();
    updateBattery();
});

const updateClock = () => {
    if (!clock) return;

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

    clock.innerHTML = "<div class='clock'>" + `${prefix || ''}${formatted}${suffix || ''}` + "</div>";
}

const updateActiveClient = () => {
    let temp = String(systemDetails.client).split("class")[1];
    let temp2 = String(systemDetails.client).replace("initial_title", "initial").split("title")[1];

    activeWindow.innerHTML = "<div class='activeWindow'>" + temp2.split("pid")[0].replaceAll('",', '').replace(': "', "").trim() + " | " + temp.replaceAll('",', '').replace(': "', "").trim() + "</div>";
}

const updateBattery = () => {
    battery.innerHTML = "<div class='battery'>" + systemDetails.battery + "</div>";
}