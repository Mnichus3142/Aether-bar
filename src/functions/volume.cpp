//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <gtk/gtk.h>
#include "../Widget.h"

static std::string exec(const char* cmd) {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "ERROR";
    }

    char buffer[128];
    std::string result = "";

    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

static gboolean update_volume(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    std::string cmd = "wpctl get-volume @DEFAULT_AUDIO_SINK@";
    std::string output = exec(cmd.c_str());

    bool isMuted = (output.find("[MUTED]") != std::string::npos);
    double vol = 0.0;

    if (isMuted) {
       gtk_label_set_text(GTK_LABEL(data->label), "0");
       if (data->icon) {
           std::string iconPath = data->iconBasePath + "volume_off.svg";
           gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
       }
       return TRUE;
    }

    size_t pos = output.find("Volume: ");
    if (pos != std::string::npos) {
        const char* start = output.c_str() + pos + 8;
        char* end = nullptr;
        vol = g_ascii_strtod(start, &end);
        int volInt = (int)(vol * 100.0);

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", volInt);
        gtk_label_set_text(GTK_LABEL(data->label), buffer);

        if (data->icon) {
            std::string iconName;
            if (volInt == 0) iconName = "volume_off";
            else if (volInt <= 33) iconName = "volume_mute";
            else if (volInt <= 66) iconName = "volume_down";
            else iconName = "volume_up";

            std::string iconPath = data->iconBasePath + iconName + ".svg";
            gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
        }
    }
    return TRUE;
}
