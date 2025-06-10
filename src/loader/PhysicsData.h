#pragma once

#include <glm/glm.hpp>

#include "physics/Category.h"
#include "physics/size.h"

namespace loader {
    // @see physics/Body.h
    struct BodyData {
        glm::vec3 size{ 1.f };

        glm::vec3 baseRotation{ 0.f };

        glm::vec3 linearVelocity{ 0.f };
        glm::vec3 angularVelocity{ 0.f };

        glm::vec3 axis{ 0.f, 1.f, 0.f };

        float maxAngulerVelocity{ 100.f };
        float density{ 1.f };

        physics::BodyType type{ physics::BodyType::none };

        bool forceAxis : 1 { false };
        bool kinematic : 1 { false };
    };

    // @see physics/Geom.h
    struct GeomData {
        bool enabled{ false };

        glm::vec3 size{ 0.5f };

        glm::vec3 rotation{ 0.f };
        glm::vec3 offset{ 0.f };

        uint32_t categoryMask{ UINT_MAX };
        uint32_t collisionMask{ UINT_MAX };

        physics::GeomType type{ physics::GeomType::none };

        bool placeable : 1 { true };
    };

    struct PhysicsData {
        bool enabled{ false };

        bool update{ false };
        BodyData body;
        GeomData geom;
    };
}
