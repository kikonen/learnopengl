#pragma once

#include "EntityCloneData.h"
#include "MaterialData.h"

namespace loader {
    struct PrefabData {
        EntityCloneData entity;
        std::vector<MaterialData> materials;
    };
}
