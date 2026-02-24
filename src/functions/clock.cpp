//
// Created by michus3142 on 24.02.2026.
//

#include <iostream>
#include <gtk/gtk.h>

static gboolean update_clock(gpointer label) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}