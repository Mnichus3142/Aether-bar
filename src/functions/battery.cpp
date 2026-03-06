//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <gtk/gtk.h>
#include "../Widget.h"

static gboolean update_battery(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    std::string status = "";
    long energy_now = -1;
    long energy_full = -1;

    std::ifstream status_file("/sys/class/power_supply/BAT0/status");
    if (status_file.is_open()) {
        std::getline(status_file, status);
        status_file.close();
    }

    std::ifstream energy_now_file("/sys/class/power_supply/BAT0/energy_now");
    if (energy_now_file.is_open()) {
        energy_now_file >> energy_now;
        energy_now_file.close();
    }

    std::ifstream energy_full_file("/sys/class/power_supply/BAT0/energy_full");
    if (energy_full_file.is_open()) {
        energy_full_file >> energy_full;
        energy_full_file.close();
    }

    // Determine status (Charging/Discharging)
    bool charging = (status == "Charging");

    int capacity = 0;
    if (energy_full > 0) {
        capacity = (int)(((double)energy_now / (double)energy_full) * 100.0);
    }

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", capacity);

    gtk_label_set_text(GTK_LABEL(data->label), buffer);

    if (data->icon) {
        std::string iconName;
        if (charging) {
            iconName = "battery_android_bolt";
        } else {
            if (capacity < 13) iconName = "battery_android_0";
            else if (capacity < 26) iconName = "battery_android_1";
            else if (capacity < 38) iconName = "battery_android_2";
            else if (capacity < 50) iconName = "battery_android_3";
            else if (capacity < 63) iconName = "battery_android_4";
            else if (capacity < 75) iconName = "battery_android_5";
            else if (capacity < 88) iconName = "battery_android_6";
            else iconName = "battery_android_full";
        }
        std::string iconPath = data->iconBasePath + iconName + ".svg";
        gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
    }

    return TRUE;
}