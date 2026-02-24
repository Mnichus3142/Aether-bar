//
// Created by michus3142 on 24.02.2026.
//

#include <gtk/gtk.h>

static void updateWidget(int interval, gboolean (*updateFunction)(gpointer), gpointer data) {
    updateFunction(data);
    g_timeout_add(interval, updateFunction, data);
}