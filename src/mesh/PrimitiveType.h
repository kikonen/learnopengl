#pragma once

namespace mesh {
    enum class PrimitiveType {
        none,
        points,
        lines,
        line_strip,
        plane,
        quad,
        box,
        sphere,
        capsule,
        cylinder,
        capped_cylinder
    };
}
