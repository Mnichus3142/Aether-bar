//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <gtk/gtk.h>
#include <fstream>

static gboolean update_battery(gpointer label) {
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

    int capacity = 0;
    if (energy_full > 0) {
        capacity = (int)(((double)energy_now / (double)energy_full) * 100.0);
    }

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", capacity);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}