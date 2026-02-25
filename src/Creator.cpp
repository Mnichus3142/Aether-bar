//
// Created by michus3142 on 18.02.2026.
//

#include "Creator.h"
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#include <fstream>
#include <iostream>
#include "spdlog/spdlog.h"
#include "Widget.h"

Creator::Creator(GtkWidget *left_zone, GtkWidget *center_zone, GtkWidget *right_zone) {
    this->left_zone = left_zone;
    this->center_zone = center_zone;
    this->right_zone = right_zone;
    this->config = this->readConfigFile();
    this->addToZones();
};

void Creator::addToZones() {
    for (Json::Value::const_iterator it = this->config.begin(); it != this->config.end(); ++it) {
        std::string zone = it.name();
        Json::Value values = *it;

        for (auto value : values) {
            if (zone == "left") {
                Widget widget(this->left_zone, value["name"].asString());
            }

            else if (zone == "center") {
                Widget widget(this->center_zone, value["name"].asString());
            }

            else if (zone == "right") {
                Widget widget(this->right_zone, value["name"].asString());
            }
        }
    }
}

Json::Value Creator::readConfigFile() {
    std::string configPath = std::string(getenv("HOME")) + "/.config/Aether-bar/config.jsonc";
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        spdlog::error("Could not open config file: {}", configPath);
        exit(1);
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, configFile, &root, &errs)) {
        spdlog::error("Error parsing config file: {}", errs);
        exit(1);
    }

    return root;
}
