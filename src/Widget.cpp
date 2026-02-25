//
// Created by michus3142 on 24.02.2026.
//

#include "Widget.h"
#include <gtk/gtk.h>
#include "functions/clock.cpp"
#include "functions/battery.cpp"
#include "functions/brightness.cpp"
#include "functions/cpu.cpp"
#include "functions/disk.cpp"
#include "functions/memory.cpp"
#include "functions/network.cpp"
#include "functions/volume.cpp"

Widget::Widget(GtkWidget *zone, std::string name) {
    GtkWidget *label = gtk_label_new("");
    gtk_widget_add_css_class(label, name.c_str() );
    gtk_box_append(GTK_BOX(zone), label);

    if (name == "clock") {
        this->updateWidget(1000, update_clock, label);
    }

    else if (name == "battery") {
        this->updateWidget(5000, update_battery, label);
    }

    else if (name == "brightness") {
        this->updateWidget(100, update_brightness, label);
    }

    else if (name == "cpu") {
        this->updateWidget(1000, update_cpu, label);
    }

    else if (name == "disk") {
        this->updateWidget(5000, update_disk, label);
    }

    else if (name == "memory") {
        this->updateWidget(5000, update_memory, label);
    }

    else if (name == "network") {
        this->updateWidget(100, update_network, label);
    }

    else if (name == "volume") {
        this->updateWidget(1000, update_volume, label);
    }
}

void Widget::updateWidget(int interval, gboolean (*updateFunction)(gpointer), gpointer data) {
    updateFunction(data);
    g_timeout_add(interval, updateFunction, data);
}
