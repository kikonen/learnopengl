#pragma once

#include <glm/glm.hpp>

#include "BaseId.h"

namespace loader {
    struct RootData {
        ki::node_id rootId;

        glm::vec3 pos{ 0.f };
    };
}
