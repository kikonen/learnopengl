#pragma once

namespace mesh {
    //
    // TODO KI missinsg from https://github.com/ilmola/generator
    // MESHES:
    // ConvexPolygonMesh
    // ParametricMesh
    // SphericalTriangleMesh
    // TeapotMesh
    // TriangleMesh
    //
    // SHAPES:
    // BezierShape
    // CircleShape
    // GridShape
    // LineShape
    // ParametricShape
    // RectangleShape
    // RoundedRectangleShape
    //
    // PATHS:
    // HelixPath
    // KnotPath
    // LinePath
    // ParametricPath
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
        icosa_hedron,
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
