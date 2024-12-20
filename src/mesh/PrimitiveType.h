#pragma once

namespace mesh {
    enum class PrimitiveType {
        none,
        points,
        lines,
        line_strip,
        ray,
        bezier,
        plane,
        quad,
        height_field,
        plane_grid,
        box,
        rounded_box,
        dodeca_hedron,
        ico_sphere,
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
