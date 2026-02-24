//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <sys/statvfs.h>
#include <gtk/gtk.h>

static gboolean update_disk(gpointer label) {
    int msg;

    struct statvfs stat;
    statvfs("/", &stat);

    float total_disk = (float)stat.f_blocks;
    float free_disk = (float)stat.f_bavail;

    msg = (int)((total_disk - free_disk) / total_disk * 100.0);

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", (int)msg);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}