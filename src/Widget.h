//
// Created by michus3142 on 24.02.2026.
//

#ifndef AETHER_BAR_WIDGET_H
#define AETHER_BAR_WIDGET_H
#include <string>
#include <gtk/gtk.h>

struct WidgetData {
    GtkWidget *label;
    GtkWidget *icon;
    std::string iconBasePath;
};

class Widget {
    public:
        Widget(GtkWidget *zone, std::string name, std::string format, std::string iconPath);
        void makeWidget(std::string name, std::string format, WidgetData *data);
        void updateWidget(int interval, gboolean (*updateFunction)(gpointer), WidgetData *data);
};


#endif //AETHER_BAR_WIDGET_H