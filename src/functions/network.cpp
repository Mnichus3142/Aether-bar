//
// Created by michus3142 on 24.02.2026.
//

#include <cstring>
#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <gtk/gtk.h>
#include "../Widget.h"

#define BAD_SIGNAL_LEVEL -110
#define GOOD_SIGNAL_LEVEL -40

static bool isInterfaceUp(const char *ifname, int sock) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
        return false;
    }

    return (ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING);
}

static int8_t getSignalStrength() {
    std::ifstream wireless_file("/proc/net/wireless");
    std::string line;

    std::getline(wireless_file, line);
    std::getline(wireless_file, line);

    while (std::getline(wireless_file, line)) {
        if (line.find("wlan0:") != std::string::npos) {
            int status;
            float link_quality, signal_level, noise_level;
            sscanf(line.c_str(), " wlan0: %d %f %f %f", &status, &link_quality, &signal_level, &noise_level);
            wireless_file.close();
            return static_cast<int8_t>(signal_level);
        }
    }

    wireless_file.close();

    return -1;
}

static float parseSignalStrengthToPercentage(int8_t signal_level) {
    return (float)(signal_level - BAD_SIGNAL_LEVEL) / (float)(GOOD_SIGNAL_LEVEL - BAD_SIGNAL_LEVEL) * 100.0;
}

static gboolean update_network(gpointer data_ptr) {
    WidgetData *data = (WidgetData*)data_ptr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int8_t msg = -1; // -1: down, 0: eth0 up, 1: wifi up
    std::string iconName = "signal_wifi_off";
    std::string text = "";

    if (isInterfaceUp("eth0", sock)) {
        msg = 0; // Ethernet up
        iconName = "lan";
        text = "Eth";
    }

    else if (isInterfaceUp("wlan0", sock)) {
        msg = 1; // WiFi up
        int signal_strength = (int)parseSignalStrengthToPercentage(getSignalStrength());
        text = std::to_string(signal_strength);

        if (signal_strength < 10) iconName = "signal_wifi_0_bar";
        else if (signal_strength < 25) iconName = "network_wifi_1_bar";
        else if (signal_strength < 50) iconName = "network_wifi_2_bar";
        else if (signal_strength < 75) iconName = "network_wifi_3_bar";
        else if (signal_strength < 90) iconName = "network_wifi";
        else iconName = "signal_wifi_4_bar";
    }

    else {
        // Network down
        msg = -1;
        text = "Disconnected";
    }

    gtk_label_set_text(GTK_LABEL(data->label), text.c_str());

    if (data->icon) {
         std::string iconPath = data->iconBasePath + iconName + ".svg";
         gtk_image_set_from_file(GTK_IMAGE(data->icon), iconPath.c_str());
    }

    return TRUE;
}
