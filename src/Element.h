//
// Created by michus3142 on 23.02.2026.
//

#ifndef AETHER_BAR_ELEMENT_H
#define AETHER_BAR_ELEMENT_H

#include <vector>

struct Element {
    std::string name;
    std::vector<std::tuple<std::string, std::string>> properties;
};

#endif //AETHER_BAR_ELEMENT_H