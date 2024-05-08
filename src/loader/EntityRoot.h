#pragma once

#include <vector>

#include "EntityCloneData.h"

namespace loader {
    struct EntityRoot {
        EntityCloneData base;
        std::vector<EntityCloneData> clones;
    };
}
