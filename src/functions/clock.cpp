//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <gtk/gtk.h>
#include "../Widget.h"

static gboolean update_clock(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);

    gtk_label_set_text(GTK_LABEL(data->label), buffer);
    if (data->icon) {
         std::string iconPath = data->iconBasePath + "clock.svg";
         gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
    }
    return TRUE;
}