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
        rounded_box,
        sphere,
        capsule,
        cylinder,
        capped_cylinder,
        cone,
        capped_cone,
        spherical_cone,
        tube,
        capped_tube,
        torus,
        torus_knot,
        disk,
        spring,
    };
}
