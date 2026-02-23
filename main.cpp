#include <gtk/gtk.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <iostream>
#include "spdlog/spdlog.h"

#include "src/Creator.h"

static void gtk_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
					 const gchar *message, gpointer user_data) {
	if (log_level & G_LOG_LEVEL_WARNING) {
		std::cout << "✨ Caught a GTK Warning: " << message << std::endl;
	}
}

// Load the CSS file from .config/Aether-bar/style.css
static void load_css() {
	// Construct the path to the CSS file
	char *css_path = g_build_filename(g_get_user_config_dir(), "Aether-bar", "style.css", NULL);
	try {
		spdlog::info("[Aether-bar] Loading CSS...");
		GtkCssProvider *provider = NULL;

		// Check if file exists
		if (!g_file_test(css_path, G_FILE_TEST_EXISTS)) {
			throw new std::runtime_error("CSS does not exist");
		}

		// Create provider for CSS
		provider = gtk_css_provider_new();

		g_signal_connect(provider, "parsing-error", G_CALLBACK(+[](GtkCssProvider *self, GtkCssSection *section, GError *error, gpointer data) {
			const GtkCssLocation *start_loc = gtk_css_section_get_start_location(section);
			spdlog::error("[Aether-bar] CSS fatal Error at line {}: {}", start_loc->lines + 1, error->message);

			exit(1);
		}), NULL);

		gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
		// Load the CSS file into the provider
		gtk_css_provider_load_from_path(provider, css_path);

		// Free the allocated path string
		g_free(css_path);
		spdlog::info("[Aether-bar] CSS Loaded");
	}

	catch (std::runtime_error *err) {
		g_free(css_path);
		spdlog::error("[Aether-bar] {}", err->what());
		delete err;
		exit(1);
	}

	catch (...) {
		spdlog::error("[Aether-bar] Unknown error occurred");
	}
}

// Monitor the CSS file for changes and reload it when it changes
static void on_css_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent ev, gpointer d) {
	spdlog::info("[Aether-bar] Reloading CSS...");
	if (ev == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT) load_css();
}

// Clock update
static gboolean update_clock(gpointer label) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}

// Callback for executing shell commands on click
static void on_click_execute(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
	spdlog::info("[Aether-bar] Executing command");
    const char *command = (const char *)user_data;
    GError *error = NULL;

    if (!g_spawn_command_line_async(command, &error)) {
        g_printerr("Error executing command: %s\n", error->message);
        g_error_free(error);
    }
}

// Helper to attach a shell command to any GtkWidget
static void attach_click_command(GtkWidget *widget, const char *command) {
    GtkGesture *gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_PRIMARY);
    g_signal_connect(gesture, "pressed", G_CALLBACK(on_click_execute), (gpointer)command);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(gesture));
}

// Activate function to set up the application window and layer shell properties
static void activate(GtkApplication *app, gpointer user_data) {
	spdlog::info("[Aether-bar] Activating Aether-bar...");
	// Create a folder in .config/Aether-bar if it doesn't exist
	char *config_dir = g_build_filename(g_get_user_config_dir(), "Aether-bar", NULL);
	g_mkdir_with_parents(config_dir, 0755);

	// Create a window and set it to be a layer shell window
	GtkWidget *window = gtk_application_window_new(app);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_layer_init_for_window(GTK_WINDOW(window));

    // Set namespace for better WM identification (helps with rules)
    gtk_layer_set_namespace(GTK_WINDOW(window), "Aether-bar");

	gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

	// Use auto-exclusive zone - more reliable in GTK4
	gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(window));

	// Create a main box
	GtkWidget *main_box = gtk_center_box_new();
    gtk_widget_set_size_request(main_box, -1, 10);
	gtk_window_set_child(GTK_WINDOW(window), main_box);

	// Crate zones
	GtkWidget *left_zone = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	GtkWidget *center_zone = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	GtkWidget *right_zone = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

	// Append zones to containers
	gtk_center_box_set_start_widget(GTK_CENTER_BOX(main_box), left_zone);
	gtk_center_box_set_center_widget(GTK_CENTER_BOX(main_box), center_zone);
	gtk_center_box_set_end_widget(GTK_CENTER_BOX(main_box), right_zone);

	Creator creator(left_zone, center_zone, right_zone);

	// // Create a clock label and add it to the right zone
	// GtkWidget *label = gtk_label_new("");
	// gtk_widget_add_css_class(label, "clock");
	// gtk_box_append(GTK_BOX(left_zone), label);
 //
 //    // Make the clock label clickable - example 1
 //    attach_click_command(label, "notify-send 'Zegar' 'Kliknięto w zegar!'");
 //
 //    // Example 2: A dedicated button to launch terminal
 //    GtkWidget *term_button = gtk_button_new_with_label("Terminal");
 //    gtk_box_append(GTK_BOX(right_zone), term_button);
 //    attach_click_command(term_button, "kitty");
 //
 //    // Clock update every second
 //    g_timeout_add(1000, update_clock, label);

    // Monitor the CSS file for changes and reload it when it changes
    load_css();
    char *css_path = g_build_filename(config_dir, "style.css", NULL);
    GFile *css_file = g_file_new_for_path(css_path);
    GFileMonitor *monitor = g_file_monitor_file(css_file, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(on_css_changed), NULL);

	spdlog::info("[Aether-bar] Aether-bar activated");

    gtk_window_present(GTK_WINDOW(window));
    g_free(config_dir); g_free(css_path);
}

// Main function to activate the application
int main(int argc, char *argv[]) {
	spdlog::info("[Aether-bar] Starting Aether-bar...");
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, gtk_log_handler, NULL);
	GtkApplication *app = gtk_application_new("com.Aether-bar", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}
