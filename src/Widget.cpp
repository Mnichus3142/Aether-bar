//
// Created by michus3142 on 24.02.2026.
//

#include "Widget.h"
#include <gtk/gtk.h>
#include "functions/updateWidget.cpp"
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
        updateWidget(1000, update_clock, label);
    }

    else if (name == "battery") {
        updateWidget(5000, update_battery, label);
    }

    else if (name == "brightness") {
        updateWidget(100, update_brightness, label);
    }

    else if (name == "cpu") {
        updateWidget(1000, update_cpu, label);
    }

    else if (name == "disk") {
        updateWidget(5000, update_disk, label);
    }

    else if (name == "memory") {
        updateWidget(5000, update_memory, label);
    }

    else if (name == "network") {
        updateWidget(100, update_network, label);
    }

    else if (name == "volume") {
        updateWidget(1000, update_volume, label);
    }
}
