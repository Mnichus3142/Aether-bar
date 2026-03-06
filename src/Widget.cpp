//
// Created by michus3142 on 24.02.2026.
//

#include "Widget.h"
#include <gtk/gtk.h>
#include "spdlog/spdlog.h"
#include "functions/clock.cpp"
#include "functions/battery.cpp"
#include "functions/brightness.cpp"
#include "functions/cpu.cpp"
#include "functions/disk.cpp"
#include "functions/memory.cpp"
#include "functions/network.cpp"
#include "functions/volume.cpp"
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

static std::string get_executable_dir() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(dirname(result));
    }
    return "";
}

Widget::Widget(GtkWidget *zone, std::string name, std::string format, std::string iconPath) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_add_css_class(box, name.c_str());
    gtk_box_append(GTK_BOX(zone), box);

    if (format.empty()) {
        format = "{}";
    }

    GtkWidget *dataLabel = nullptr;
    GtkWidget *iconImage = nullptr;
    size_t pos = 0;

    // Dynamically set ICON_BASE_PATH
    std::string iconBasePath = get_executable_dir() + "/static/svg/";

    while (pos < format.length()) {
        size_t openBrace = format.find('{', pos);

        if (openBrace == std::string::npos) {
            std::string text = format.substr(pos);
            if (!text.empty()) {
                GtkWidget *label = gtk_label_new(text.c_str());
                gtk_box_append(GTK_BOX(box), label);
            }
            break;
        }

        if (openBrace > pos) {
            std::string text = format.substr(pos, openBrace - pos);
            GtkWidget *label = gtk_label_new(text.c_str());
            gtk_box_append(GTK_BOX(box), label);
        }

        size_t closeBrace = format.find('}', openBrace);
        if (closeBrace == std::string::npos) {
            std::string text = format.substr(openBrace);
            GtkWidget *label = gtk_label_new(text.c_str());
            gtk_box_append(GTK_BOX(box), label);
            break;
        }

        std::string token = format.substr(openBrace, closeBrace - openBrace + 1);

        if (token == "{}") {
            dataLabel = gtk_label_new("");
            gtk_box_append(GTK_BOX(box), dataLabel);
        } else if (token == "{ico}") {
            iconImage = gtk_image_new();
            if (!iconPath.empty()) {
                 gtk_image_set_from_file(GTK_IMAGE(iconImage), iconPath.c_str());
            }
            gtk_box_append(GTK_BOX(box), iconImage);
        } else {
            GtkWidget *label = gtk_label_new(token.c_str());
            gtk_box_append(GTK_BOX(box), label);
        }

        pos = closeBrace + 1;
    }

    if (!dataLabel) {
        dataLabel = gtk_label_new("");
        gtk_widget_set_visible(dataLabel, FALSE);
        gtk_box_append(GTK_BOX(box), dataLabel);
    }

    WidgetData *data = new WidgetData{dataLabel, iconImage, iconBasePath};

    this->makeWidget(name, format, data);
}

void Widget::makeWidget(std::string name, std::string format, WidgetData *data) {
    if (name == "clock") {
        this->updateWidget(1000, update_clock, data);
    }

    else if (name == "battery") {
        this->updateWidget(5000, update_battery, data);
    }

    else if (name == "brightness") {
        this->updateWidget(100, update_brightness, data);
    }

    else if (name == "cpu") {
        this->updateWidget(1000, update_cpu, data);
    }

    else if (name == "disk") {
        this->updateWidget(5000, update_disk, data);
    }

    else if (name == "memory") {
        this->updateWidget(5000, update_memory, data);
    }

    else if (name == "network") {
        this->updateWidget(100, update_network, data);
    }

    else if (name == "volume") {
        this->updateWidget(1000, update_volume, data);
    }
}

void Widget::updateWidget(int interval, gboolean (*updateFunction)(gpointer), WidgetData *data) {
    updateFunction(data);
    g_timeout_add(interval, updateFunction, data);
}
