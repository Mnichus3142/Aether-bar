//
// Created by michus3142 on 18.02.2026.
//

#include "Creator.h"
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "spdlog/spdlog.h"

Creator::Creator(GtkWidget *left_zone, GtkWidget *center_zone, GtkWidget *right_zone) {
    this->left_zone = left_zone;
    this->center_zone = center_zone;
    this->right_zone = right_zone;
    this->config = this->readConfigFile();
    this->addToZones();
};

void Creator::addToZones() {
    for (Json::Value::const_iterator it = this->config.begin(); it != this->config.end(); ++it) {
        std::string key = it.name();
        Json::Value value = *it;

        for (Json::Value::const_iterator ot = value.begin(); ot != value.end(); ++ot) {
            std::string name = ot.name();
            Json::Value value2 = *ot;

            this->addToZone(key, name, value2);
        }
    }
}

void Creator::addToZone(std::string zone, std::string name, Json::Value value) {
    Element el;
    el.name = name;

    for (Json::Value::const_iterator it = value.begin(); it != value.end(); ++it) {
        std::string key = it.name();
        std::string value = it->toStyledString();
        value.erase(value.begin(), value.begin() + 1);
        value.erase(value.end() - 2, value.end() - 1);

        el.properties.push_back({key, value});
    }

    if (zone == "left") {
        this->zones.Left.push_back(el);
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
