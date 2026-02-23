//
// Created by michus3142 on 18.02.2026.
//

#ifndef AETHER_BAR_CREATOR_H
#define AETHER_BAR_CREATOR_H
#include <string>
#include <gtk/gtk.h>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#include "Zones.h"

class Creator {
    private:
        GtkWidget *left_zone;
        GtkWidget *center_zone;
        GtkWidget *right_zone;
        Json::Value config;
        Zones zones;

    public:
        Creator(GtkWidget *left_zone, GtkWidget *center_zone, GtkWidget *right_zone);
        void addToLeftZone(GtkWidget *widget);
        void addToCenterZone(GtkWidget *widget);
        void addToRightZone(GtkWidget *widget);
        Json::Value readConfigFile();
        void addToZones();
        void addToZone(std::string zone, std::string name, Json::Value value);
};


#endif //AETHER_BAR_CREATOR_H