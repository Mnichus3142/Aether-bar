//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <fstream>
#include <sys/statvfs.h>
#include <gtk/gtk.h>
#include "../Widget.h"

static gboolean update_disk(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    int msg;

    struct statvfs stat;
    statvfs("/", &stat);

    float total_disk = (float)stat.f_blocks;
    float free_disk = (float)stat.f_bavail;

    msg = (int)((total_disk - free_disk) / total_disk * 100.0);

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", (int)msg);

    gtk_label_set_text(GTK_LABEL(data->label), buffer);

    if (data->icon) {
         std::string iconPath = data->iconBasePath + "disk.svg";
         gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
    }
    return TRUE;
}