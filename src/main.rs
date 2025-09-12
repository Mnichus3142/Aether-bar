use anyhow::Result;
use gio::prelude::*;
use gtk4::prelude::*;
use gtk4::prelude::{ApplicationExt, ApplicationExtManual};
use gtk4::{Application, ApplicationWindow};
use gtk4::CssProvider;
use gtk4::gdk;
use gtk4_layer_shell as gls;
use gls::LayerShell; // metody layer-shell jako trait
use gls::KeyboardMode;
use std::path::PathBuf;
use gio::File;
// Zmiana: używamy webkit6 (zgodnego z GTK4)
use webkit6::prelude::*; // WebViewExt/WebViewExtManual
use webkit6::WebView;

const APP_ID: &str = "dev.example.hyprbar";
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

fn build_bar(app: &Application) -> ApplicationWindow {
    // Layer-shell window
    let win = ApplicationWindow::builder()
        .application(app)
        .title("HyprBar")
        .decorated(false)
        .resizable(false)
        .build();

    // Globalny CSS: brak tła okna GTK
    let provider = CssProvider::new();
    let _ = provider.load_from_data("window, .background { background-color: transparent; }");
    if let Some(display) = gdk::Display::default() {
        gtk4::style_context_add_provider_for_display(
            &display,
            &provider,
            gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );
    }
    
    // Layer Shell konfiguracja przez trait
    win.init_layer_shell();
    win.set_layer(gls::Layer::Top);
    win.set_anchor(gls::Edge::Top, true);
    win.set_anchor(gls::Edge::Left, true);
    win.set_anchor(gls::Edge::Right, true);
    win.auto_exclusive_zone_enable();
    // Zmieniono nazwę i enum w gtk4-layer-shell 0.4
    win.set_keyboard_mode(KeyboardMode::OnDemand);

    // Height drives the exclusive zone
    win.set_size_request(-1, BAR_HEIGHT);
    win.set_default_size(800, BAR_HEIGHT);

    // Web UI (HTML/CSS/JS via WebKitGTK)
    let webview = WebView::new();
    // Przezroczyste tło WebView
    let rgba = gdk::RGBA::new(0.0, 0.0, 0.0, 0.0);
    webview.set_background_color(&rgba);
    webview.set_hexpand(true);
    webview.set_vexpand(true);

    // Load static/index.html
    let static_dir = find_static_dir();
    let index_path = static_dir.join("index.html");
    let index_uri = File::for_path(&index_path).uri();
    eprintln!("[Aether] Loading URI: {}", index_uri);
    webview.load_uri(&index_uri);

    // Loguj etapy ładowania i ewentualne błędy
    webview.connect_load_changed(|_wv, ev| {
        eprintln!("[Aether] WebView load_changed: {:?}", ev);
    });
    webview.connect_load_failed(|_wv, ev, _failing_uri, error| {
        eprintln!("[Aether] WebView load_failed at {:?}: {}", ev, error);
        false
    });

    // Example: send a clock tick into the web UI every second
    {
        let webview = webview.clone();
        glib::timeout_add_seconds_local(1, move || {
            let now = chrono::Local::now().format("%H:%M:%S").to_string();
            let js = format!(
                "window.dispatchEvent(new CustomEvent('tick', {{ detail: {{ time: '{}' }} }}));",
                now.replace('\'', "\\'")
            );
            // webkit6 0.4: evaluate_javascript(script, world_name, source_uri, cancellable, callback)
            webview.evaluate_javascript(
                &js,
                None::<&str>,
                None::<&str>,
                None::<&gtk4::gio::Cancellable>,
                |_| {},
            );
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