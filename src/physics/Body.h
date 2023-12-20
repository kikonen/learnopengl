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
        BodyType type{ BodyType::none };

        bool kinematic{ false };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        float density{ 1.f };

        // initial values for physics
        glm::vec3 linearVel{ 0.f };
        glm::vec3 angularVel{ 0.f };

        // NOTE KI *ROTATED* using rotation of node
        // axis + angle
        glm::quat quat{ 1.f, 0.f, 0.f, 0.f };
    };
}
