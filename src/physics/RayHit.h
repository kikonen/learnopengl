#pragma once

#include <glm/glm.hpp>

#include "pool/NodeHandle.h"

namespace physics {
    struct RayHit {
        pool::NodeHandle handle;
        glm::vec3 pos{ 0.f };
        glm::vec3 normal{ 0.f };
        float depth{ 0.f };
    };
}
