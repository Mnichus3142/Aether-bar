use anyhow::Result;
use gio::prelude::*;
use gtk4::{prelude::*};
use gtk4::prelude::{ApplicationExt, ApplicationExtManual};
use gtk4::{Application, ApplicationWindow};
use gtk4::CssProvider;
use gtk4::gdk;
use gtk4_layer_shell as gls;
use gls::LayerShell;
use gls::KeyboardMode;
use std::path::PathBuf;
use gio::File;
use std::fs;
use webkit6::prelude::*;
use webkit6::WebView;
use hyprland::data::{Client, Workspace, Workspaces};
use hyprland::shared::{HyprData, HyprDataActive, HyprDataActiveOptional};
use battery::Manager as BatteryManager;
use machine_info::Machine;
use nix::sys::statvfs::statvfs;
use std::path::Path;

const APP_ID: &str = "dev.example.aether";
const BAR_HEIGHT: i32 = 32;

fn find_static_dir() -> PathBuf {
    #[cfg(debug_assertions)]
    {
        PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("static")
    }
    #[cfg(not(debug_assertions))]
    {
        std::env::current_exe()
            .ok()
            .and_then(|p| p.parent().map(|d| d.to_path_buf()))
            .unwrap_or_else(|| PathBuf::from("."))
            .join("static")
    }
}

fn read_brightness_percent() -> Option<i32> {
    let dir = Path::new("/sys/class/backlight");
    let entries = std::fs::read_dir(dir).ok()?;

    let mut chosen: Option<PathBuf> = None;
    for entry in entries.flatten() {
        let name = entry.file_name().to_string_lossy().to_string();
        let path = entry.path();
        if name.contains("intel") || name.contains("amdgpu") || name.contains("nvidia") {
            chosen = Some(path);
            break;
        }
        if chosen.is_none() {
            chosen = Some(path);
        }
    }

    let base = chosen?;
    let cur = std::fs::read_to_string(base.join("brightness")).ok()?;
    let max = std::fs::read_to_string(base.join("max_brightness")).ok()?;
    let cur: u64 = cur.trim().parse().ok()?;
    let max: u64 = max.trim().parse().ok()?;
    if max == 0 { return None; }
    let pct = ((cur as f64 / max as f64) * 100.0).round() as i32;
    Some(pct.clamp(0, 100))
}

fn build_bar(app: &Application) -> ApplicationWindow {
    let win = ApplicationWindow::builder()
        .application(app)
        .title("Aether")
        .decorated(false)
        .resizable(false)
        .build();

    let provider = CssProvider::new();
    let _ = provider.load_from_data("window, .background { background-color: transparent; }");
    if let Some(display) = gdk::Display::default() {
        gtk4::style_context_add_provider_for_display(
            &display,
            &provider,
            gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );
    }
    
    win.init_layer_shell();
    win.set_layer(gls::Layer::Top);
    win.set_anchor(gls::Edge::Top, true);
    win.set_anchor(gls::Edge::Left, true);
    win.set_anchor(gls::Edge::Right, true);
    win.auto_exclusive_zone_enable();

    win.set_keyboard_mode(KeyboardMode::OnDemand);

    if gls::is_supported() {
        eprintln!("[Aether] layer-shell supported: anchors Left+Right -> full width");
        win.set_size_request(-1, BAR_HEIGHT);
        if let Some(display) = gdk::Display::default() {
            let monitors = display.monitors();
            let width = (0..monitors.n_items())
                .find_map(|i| monitors.item(i))
                .and_then(|obj| obj.downcast::<gdk::Monitor>().ok())
                .map(|m| m.geometry().width())
                .unwrap_or(800);
            win.set_default_size(width, BAR_HEIGHT);
        } else {
            win.set_default_size(800, BAR_HEIGHT);
        }
    } else {
        eprintln!("[Aether] layer-shell NOT supported: using normal window fallback");
        if let Some(display) = gdk::Display::default() {
            let monitors = display.monitors();
            let width = (0..monitors.n_items())
                .find_map(|i| monitors.item(i))
                .and_then(|obj| obj.downcast::<gdk::Monitor>().ok())
                .map(|m| m.geometry().width())
                .unwrap_or(800);
            win.set_default_size(width, BAR_HEIGHT);
        } else {
            win.set_default_size(800, BAR_HEIGHT);
        }
    }

    let webview = WebView::new();

    if let Some(settings) = webkit6::prelude::WebViewExt::settings(&webview) {
        settings.set_enable_javascript(true);
        settings.set_enable_developer_extras(true);
    }

    let rgba = gdk::RGBA::new(0.0, 0.0, 0.0, 0.0);
    webview.set_background_color(&rgba);
    webview.set_hexpand(true);
    webview.set_vexpand(true);

    let static_dir = find_static_dir();

    let user_config_dir = glib::user_config_dir().join("aether");
    let user_css_path = user_config_dir.join("style.css");
    let mut user_css_exists = user_css_path.exists();

    if !user_css_exists {
        let default_css_path = static_dir.join("styles.css");
        match fs::read_to_string(&default_css_path) {
            Ok(default_css) => {
                if let Err(e) = fs::create_dir_all(&user_config_dir) {
                    eprintln!("[Aether] Could not create config directory: {}", e);
                }
                match fs::write(&user_css_path, default_css) {
                    Ok(_) => {
                        eprintln!("[Aether] Created default ~/.config/aether/style.css");
                        user_css_exists = true;
                    }
                    Err(e) => eprintln!("[Aether] Could not save user style: {}", e),
                }
            }
            Err(e) => eprintln!("[Aether] Could not read default CSS: {}", e),
        }
    }

    let user_css_uri: String = File::for_path(&user_css_path).uri().to_string();

    let user_config_json_path = user_config_dir.join("config.json");
    if !user_config_json_path.exists() {
        let default_cfg_path = static_dir.join("config.json");
        match fs::read_to_string(&default_cfg_path) {
            Ok(default_cfg) => {
                if let Err(e) = fs::create_dir_all(&user_config_dir) {
                    eprintln!("[Aether] Could not create config directory: {}", e);
                }
                if let Err(e) = fs::write(&user_config_json_path, default_cfg) {
                    eprintln!("[Aether] Could not save user config: {}", e);
                } else {
                    eprintln!("[Aether] Created default ~/.config/aether/config.json");
                }
            }
            Err(e) => eprintln!("[Aether] Could not read default config.json: {}", e),
        }
    }

    let mut injected_config_json = String::from("{}");
    if let Ok(cfg) = fs::read_to_string(&user_config_json_path) {
        injected_config_json = cfg;
    }

    let index_path = static_dir.join("index.html");
    let index_uri = File::for_path(&index_path).uri();
    eprintln!("[Aether] Loading URI: {}", index_uri);

    {
        let user_css_uri = user_css_uri.clone();
        let user_css_exists_captured = user_css_exists;
        let injected_config_json = injected_config_json.clone();
        webview.connect_load_changed(move |wv, ev| {
            eprintln!("[Aether] WebView load_changed: {:?}", ev);
            if matches!(ev, webkit6::LoadEvent::Committed | webkit6::LoadEvent::Finished) {
                if user_css_exists_captured {
                    let js = format!(
                        r#"(function(){{
  try {{
    const head = document.head || document.getElementsByTagName('head')[0];
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.type = 'text/css';
    link.href = '{}';
    head.appendChild(link);
  }} catch (e) {{
    console.error('Inject CSS failed', e);
  }}
}})();"#,
                        user_css_uri.replace('\'', "\\'")
                    );
                    wv.evaluate_javascript(
                        &js,
                        None::<&str>,
                        None::<&str>,
                        None::<&gtk4::gio::Cancellable>,
                        |_| {},
                    );
                }

                let js_cfg = format!(
                    r#"(function(){{
  try {{
    window.AetherConfig = Object.freeze({});
    window.dispatchEvent(new CustomEvent('config', {{ detail: window.AetherConfig }}));
  }} catch (e) {{ console.error('Inject config failed', e); }}
}})();"#,
                    injected_config_json
                );

                wv.evaluate_javascript(
                    &js_cfg,
                    None::<&str>,
                    None::<&str>,
                    None::<&gtk4::gio::Cancellable>,
                    |_| {},
                );
            }
        });
    }

    webview.load_uri(&index_uri);

    {
        let mut machine = Machine::new();

        let webview = webview.clone();
        let mut last_time = String::new();
        let mut last_client = String::new();
        let mut last_battery = String::new();
        let mut last_battery_state = String::new();
        let mut last_workspace = String::new();
        let mut last_workspaces = String::new();
        let mut last_cpu_util = 0;
        let mut last_memory_used = 0;
        let mut last_disk_usage: i32 = 0;
        let mut last_brightness: i32 = 0;

        glib::timeout_add_local(std::time::Duration::from_millis(150), move || {
            let now_time = chrono::Local::now().format("%H:%M:%S").to_string();
            let mut updates: Vec<String> = Vec::new();
            if now_time != last_time {
                updates.push(format!("time: '{}'", now_time.replace('\'', "\\'")));
                last_time = now_time;
            }

            if let Some(pct) = read_brightness_percent() {
                if pct != last_brightness {
                    updates.push(format!("brightness: '{}%'", pct));
                    last_brightness = pct;
                }
            }

            let machine_status = machine.system_status().unwrap();
            let cpu_usage = machine_status.cpu;
            let memory_usage = machine_status.memory;

            let machine_info = machine.system_info();

            let total_memory = machine_info.memory as f64 * (10f64.powi(-9));
            let used_memory = memory_usage as f64 * (10f64.powi(-6));
            let memory_percentage = (used_memory / total_memory * 100.0).round() as i32;

            if memory_percentage != last_memory_used {
                updates.push(format!("memory_usage: '{}%'", memory_percentage));
                last_memory_used = memory_percentage;
            }

            if cpu_usage != last_cpu_util {
                updates.push(format!("cpu_usage: '{}%'", cpu_usage));
                last_cpu_util = cpu_usage;
            }

            let disk_usage_pct: i32 = match statvfs(Path::new("/")) {
                Ok(s) => {
                    let bsize = s.block_size() as u64;
                    let total = s.blocks() * bsize;
                    let avail = s.blocks_available() * bsize;
                    if total == 0 {
                        0
                    } else {
                        (((total - avail) as f64 / total as f64) * 100.0).round() as i32
                    }
                }
                Err(_) => -1,
            };
            
            if disk_usage_pct >= 0 && disk_usage_pct != last_disk_usage {
                updates.push(format!("disk_usage: '{}%'", disk_usage_pct));
                last_disk_usage = disk_usage_pct;
            }

            let client_data = match Client::get_active() {
                Ok(opt) => {
                    if let Some(client) = opt {
                        format!("{:?}", client)
                    } else {
                        String::new()
                    }
                }
                Err(e) => format!("Error: {:?}", e),
            };

            if client_data != last_client {
                updates.push(format!("client: '{}'", client_data.replace('\'', "\\'")));
                last_client = client_data;
            }

            let workspace = match Workspace::get_active() {
                Ok(ws) => format!("{:?}", ws),
                Err(e) => format!("Error: {:?}", e),
            };
            
            let workspaces = match Workspaces::get() {
                Ok(ws) => format!("{:?}", ws),
                Err(e) => format!("Error: {:?}", e),
            };

            if workspace != last_workspace || workspaces != last_workspaces {
                updates.push(format!("workspace: '{}'", workspace.replace('\'', "\\'")));
                updates.push(format!("workspaces: '{}'", workspaces.replace('\'', "\\'")));
                last_workspace = workspace;
                last_workspaces = workspaces;
            }

            let (battery_percent, battery_state_text) = {
                let manager = BatteryManager::new();
                match manager {
                    Ok(m) => {
                        let batteries_result = m.batteries();
                        match batteries_result {
                            Ok(mut iter) => {
                                if let Some(Ok(battery)) = iter.next() {
                                    (
                                        format!("{:.0}%", battery.state_of_charge().value * 100.0),
                                        format!("{:?}", battery.state()),
                                    )
                                } else {
                                    ("No Battery".to_string(), "Unknown".to_string())
                                }
                            },
                            Err(e) => {
                                ("Error".to_string(), format!("Error: {:?}", e))
                            },
                        }
                    }
                    Err(e) => ("Error".to_string(), format!("Error: {:?}", e)),
                }
            };

            if battery_percent != last_battery {
                updates.push(format!("battery: '{}'", battery_percent.replace('\'', "\\'")));
                last_battery = battery_percent;
            }
            if battery_state_text != last_battery_state {
                updates.push(format!("battery_state: '{}'", battery_state_text.replace('\'', "\\'")));
                last_battery_state = battery_state_text;
            }

            if !updates.is_empty() {
                let payload = updates.join(", ");
                let js = format!(
                    "window.dispatchEvent(new CustomEvent('tick', {{ detail: {{ {} }} }}));",
                    payload
                );
                webview.evaluate_javascript(
                    &js,
                    None::<&str>,
                    None::<&str>,
                    None::<&gtk4::gio::Cancellable>,
                    |_| {},
                );
            }

            glib::ControlFlow::Continue
        });
    }

    win.set_child(Some(&webview));
    win
}

fn main() -> Result<()> {

    let app = Application::builder().application_id(APP_ID).build();
    app.connect_activate(|app| {
        let win = build_bar(app);
        win.present();
    });
    app.run();
    Ok(())
}