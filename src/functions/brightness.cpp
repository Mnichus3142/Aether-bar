//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <gtk/gtk.h>
#include "../Widget.h"

static gboolean update_brightness(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    int brightness_value = 0;
    std::ifstream brightness_file("/sys/class/backlight/intel_backlight/brightness");
    if (!brightness_file.is_open()) {
        gtk_label_set_text(GTK_LABEL(data->label), "err");
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

    gtk_label_set_text(GTK_LABEL(data->label), buffer);

    if (data->icon) {
        std::string iconName;
        if (brightness_value < 33) iconName = "brightness_low";
        else if (brightness_value < 66) iconName = "brightness_medium";
        else iconName = "brightness_high";

        std::string iconPath = data->iconBasePath + iconName + ".svg";
        gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
    }

    return TRUE;
}