#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace physics {
    enum class BodyType : std::underlying_type_t<std::byte> {
        none,
        box,
        sphere,
        capsule,
        cylinder,
    };

    struct Body {
        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        // box = x, y, z
        // capsule/cylinder = radiux, half-length
        glm::vec3 size{ 1.f };

        glm::vec3 scale{ 1.f };

        // initial values for physics
        glm::vec3 linearVelocity{ 0.f };
        glm::vec3 angularVelocity{ 0.f };

        // default = dInfinity
        // 0 = don't allow rotate
        float maxAngulerVelocity{ 100.f };

        // NOTE KI base rotation to match base rotation of node
        glm::quat baseRotation{ 1.f, 0.f, 0.f, 0.f };
        // reverse rotation of baseRotation
        glm::quat invBaseRotation{ 1.f, 0.f, 0.f, 0.f };

        glm::vec3 axis{ 0.f, 1.f, 0.f };
        bool forceAxis{ false };

        float density{ 1.f };

        BodyType type{ BodyType::none };

        bool kinematic : 1{ false };
    };
}
