//
// Created by michus3142 on 23.02.2026.
//

#ifndef AETHER_BAR_ZONES_H
#define AETHER_BAR_ZONES_H

#include <vector>

#include "Element.h"


struct Zones {
    std::vector<Element> Left;
    std::vector<Element> Center;
    std::vector<Element> Right;
};

#endif //AETHER_BAR_ZONES_H