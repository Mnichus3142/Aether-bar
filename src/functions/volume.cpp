//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <gtk/gtk.h>

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

static gboolean update_volume(gpointer label) {
    std::string cmd = "wpctl get-volume @DEFAULT_AUDIO_SINK@";
    std::string output = exec(cmd.c_str());

    if (output.find("[MUTED]") != std::string::npos) {
        gtk_label_set_text(GTK_LABEL(label), "0");
        return TRUE;
    }

    size_t pos = output.find("Volume: ");
    if (pos != std::string::npos) {
        const char* start = output.c_str() + pos + 8;
        char* end = nullptr;
        double vol = g_ascii_strtod(start, &end);

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "1|%.0f", vol * 100.0);
        gtk_label_set_text(GTK_LABEL(label), buffer);
    }
    return TRUE;
}
