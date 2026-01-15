#pragma once

#include <glm/glm.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "pool/NodeHandle.h"

namespace physics {
    struct GeomHit {
        glm::vec3 pos{ 0.f };
        glm::vec3 normal{ 0.f };
        JPH::BodyID bodyId;
        float depth{ 0.f };
    };

    struct RayHit {
        glm::vec3 pos{ 0.f };
        glm::vec3 normal{ 0.f };
        pool::NodeHandle handle;
        float depth{ 0.f };
        bool isHit{ false };
    };
}
