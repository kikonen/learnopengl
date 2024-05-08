#pragma once

#include "EntityData.h"
#include "MaterialData.h"

namespace loader {
    struct PrefabData {
        EntityCloneData entity;
        std::vector<MaterialData> materials;
    };
}
