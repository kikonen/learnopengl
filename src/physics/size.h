#pragma once

#include <stdint.h>

namespace physics
{
    typedef uint32_t object_id;
    typedef uint16_t height_map_id;
    typedef uint16_t material_id;

    enum class GeomType : std::underlying_type_t<std::byte> {
        none = 0,
        ray,
        plane,
        height_field,
        box,
        sphere,
        capsule,
        cylinder,
    };

    enum class BodyType : std::underlying_type_t<std::byte> {
        none = 0,
        box,
        sphere,
        capsule,
        cylinder,
    };
}
