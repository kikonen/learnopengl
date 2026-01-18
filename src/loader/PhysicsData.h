#pragma once

#include <glm/glm.hpp>

#include "util/Axis.h"

#include "physics/Category.h"
#include "physics/size.h"

namespace loader {
    // @see physics/Body.h
    struct BodyData {
        glm::vec3 size{ 1.f };

        util::Axis baseAxis{ util::Axis::y };
        util::Front baseFront{ util::Front::z };
        glm::vec3 baseAdjust{ 0.f };

        glm::vec3 linearVelocity{ 0.f };
        glm::vec3 angularVelocity{ 0.f };

        glm::vec3 axis{ 0.f, 1.f, 0.f };

        float maxAngulerVelocity{ 100.f };
        float density{ 1.f };

        physics::BodyType type{ physics::BodyType::none };

        bool forceAxis : 1 { false };
        bool kinematic : 1 { false };
    };

    // @see physics/Shape.h
    struct ShapeData {
        bool enabled{ false };

        glm::vec3 size{ 0.5f };

        util::Axis baseAxis{ util::Axis::y };
        util::Front baseFront{ util::Front::z };
        glm::vec3 baseAdjust{ 0.f };
        glm::vec3 offset{ 0.f };

        physics::Category category{ physics::Category::none };
        uint32_t collisionMask{ UINT_MAX };

        physics::ShapeType type{ physics::ShapeType::none };

        bool placeable : 1 { true };
    };

    struct PhysicsData {
        bool enabled{ false };

        bool update{ false };
        BodyData body;
        ShapeData shape;
    };
}
