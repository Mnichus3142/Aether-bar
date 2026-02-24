//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <sys/statvfs.h>
#include <gtk/gtk.h>

static gboolean update_memory(gpointer label) {
    int msg;

    std::ifstream proc_mem("/proc/meminfo");
    std::string line;
    int64_t total_mem = 0;
    int64_t free_mem = 0;

    while (std::getline(proc_mem, line)) {
        if (line.find("MemTotal:") == 0) {
            sscanf(line.c_str(), "MemTotal: %ld kB", &total_mem);
        } else if (line.find("MemAvailable:") == 0) {
            sscanf(line.c_str(), "MemAvailable: %ld kB", &free_mem);
            break;
        }
    }

    proc_mem.close();

    msg = (int)((double)(total_mem - free_mem) / (double)total_mem * 100.0);

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", (int)msg);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}