#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace physics {
    enum class GeomType : std::underlying_type_t<std::byte> {
        none,
        plane,
        box,
        sphere,
        capsule,
        cylinder,
    };

    struct Geom {
        GeomType type{ GeomType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        glm::quat quat{ 1.f, 0.f, 0.f, 0.f };

        glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        unsigned int category{ UINT_MAX };
        unsigned int collide{ UINT_MAX };
    };

}
