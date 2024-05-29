#pragma once

#include "EntityData.h"
#include "MaterialData.h"

namespace loader {
    struct PrefabData {
        EntityData entity;
        std::vector<MaterialData> materials;
    };
}
