//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <gtk/gtk.h>

static gboolean update_brightness(gpointer label) {
    int brightness_value = 0;
    std::ifstream brightness_file("/sys/class/backlight/intel_backlight/brightness");
    if (!brightness_file.is_open()) {
        gtk_label_set_text(GTK_LABEL(label), "err");
        return TRUE;
    }
    brightness_file >> brightness_value;
    brightness_file.close();

    brightness_value /= 1000;

    int full = brightness_value / 5;
    int remainings = brightness_value % 5;

    brightness_value = full * 5 + (remainings > 0 ? 5 : 0);
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", brightness_value);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}