//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <gtk/gtk.h>

static gboolean update_cpu(gpointer label) {
    static int32_t prev_total_jiffies = 0, prev_work_jiffies = 0;
    int32_t total_jiffies = 0, work_jiffies = 0;

    std::ifstream proc_stat("/proc/stat");
    std::string line;

    if (proc_stat >> line && line == "cpu") {
        for (int i = 0; i < 10; ++i) {
            int32_t value;
            proc_stat >> value;
            total_jiffies += value;
            if (i < 3) {
                work_jiffies += value;
            }
        }
    }
    proc_stat.close();

    double cpu_usage = 0.0;
    if (prev_total_jiffies != 0) {
        int32_t total_diff = total_jiffies - prev_total_jiffies;
        int32_t work_diff = work_jiffies - prev_work_jiffies;
        if (total_diff > 0) {
            cpu_usage = 100.0 * (double)work_diff / (double)total_diff;
        }
    }

    prev_total_jiffies = total_jiffies;
    prev_work_jiffies = work_jiffies;

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", (int)cpu_usage);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}