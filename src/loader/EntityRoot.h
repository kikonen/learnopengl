#pragma once

#include <vector>

#include "EntityData.h"

namespace loader {
    struct EntityRoot {
        EntityData base;
        std::vector<EntityData> clones;
    };
}
