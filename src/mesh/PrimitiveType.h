#pragma once

namespace mesh {
    enum class PrimitiveType {
        none,
        points,
        lines,
        line_strip,
        ray,
        plane,
        quad,
        box,
        sphere,
        capsule,
        cylinder,
        capped_cylinder,
        cone,
        capped_cone,
        torus,
        disk,
        spring,
    };
}
