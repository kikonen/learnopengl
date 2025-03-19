#pragma once

#include <glm/glm.hpp>

#include "ode/ode.h"

#include "pool/NodeHandle.h"

namespace physics {
    struct GeomHit {
        glm::vec3 pos{ 0.f };
        glm::vec3 normal{ 0.f };
        dGeomID geomId{ nullptr };
        float depth{ 0.f };
    };

    struct RayHit {
        glm::vec3 pos{ 0.f };
        glm::vec3 normal{ 0.f };
        pool::NodeHandle handle;
        float depth{ 0.f };
    };
}
