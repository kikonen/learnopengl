#pragma once

#include "NodeData.h"
#include "MaterialData.h"

namespace loader {
    struct PrefabData {
        NodeData node;
        std::vector<MaterialData> materials;
    };
}
