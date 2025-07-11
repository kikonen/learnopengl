#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "mesh/PrimitiveType.h"

namespace mesh {
    class Mesh;

    struct PrimitiveGenerator {
        PrimitiveType type;
        std::string name;
        std::string alias;

        glm::vec3 size{ 0.5f, 0.5f, 0.5f };
        float inner_radius = 0.f;
        float radius = 0.5f;
        float length{ 0.5f };
        int p{ 0 };
        int q{ 0 };
        int slices{ 32 };
        glm::ivec3 segments{ 4 };
        glm::ivec3 sub_segments{ 2 };
        int rings{ 8 };

        glm::vec3 origin{ 0.f };
        glm::vec3 dir{ 0.f, 0.f, -1.f };

        std::vector<glm::vec3> bezier_d0;
        std::vector<glm::vec3> bezier_d1;

        std::vector<glm::vec3> vertices;
        std::vector<int> indeces;

        // NOTE KI height map data
        // - size{x, z} world size for height map
        // - data is assumed to already have proper Y value in coords matching size
        int heightSamplesWidth{ 0 };
        int heightSamplesDepth{ 0 };
        const float* heightData{ nullptr };


        static PrimitiveGenerator none()
        {
            return {
                .type = PrimitiveType::none,
                .name = "<none>",
                .alias = "none"
            };
        }

        static PrimitiveGenerator points()
        {
            return {
                .type = PrimitiveType::points,
                .name = "<points>",
                .alias = "points"
            };
        }

        static PrimitiveGenerator lines()
        {
            return {
                .type = PrimitiveType::lines,
                .name = "<lines>",
                .alias = "lines"
            };
        }

        /// @param origin origin of ray
        /// @param dir dir of ray
        /// @param length length of ray
        static PrimitiveGenerator ray()
        {
            return {
                .type = PrimitiveType::ray,
                .name = "<ray>",
                .alias = "ray"
            };
        }

        // NOTE KI 4 x 1 bezier control points
        // - passed via bezier_d0 and bezier_d1
        //
        //vertex:
        //  type: bezier
        //  bezier_d0:
        //    - [0, 0, 0]
        //    - [1, 1, 1]
        //  bezier_d1:
        //    - [2, 0, 0]
        //    - [1, -1, 1]
        //
        // https://se.mathworks.com/matlabcentral/fileexchange/42302-smooth-3d-bezier-curves-with-implicit-control-points?s_tid=blogs_rc_5
        //
        /// A bezier patch with D0xD1 control points.
        /// @tparam D0 Number of control points along the t[0] axis. Must be > 1.
        /// @tparam D1 Number of control points along the t[1] axis. Must be > 1.
        /// @image html BezierMesh.svg
        static PrimitiveGenerator bezier()
        {
            return {
                .type = PrimitiveType::bezier,
                .name = "<bezier>",
                .alias = "bezier",
                .segments = { 2, 2, 2 }
            };
        }

        static PrimitiveGenerator plane()
        {
            // TODO KI 0.5 would better base size, but may existing assumptions
            return {
                .type = PrimitiveType::plane,
                .name = "<plane>",
                .alias = "plane",
                //.size{ 0.5f, 0.5f, 0.5f },
                // NOTE KI by default just one segement needed
                .segments{ 1, 1, 1 }
            };
        }

        static PrimitiveGenerator quad()
        {
            // TODO KI 0.5 would better base size, but may existing assumptions
            return {
                .type = PrimitiveType::quad,
                .name = "<quad>",
                .alias = "quad",
                //.size{ 0.5f, 0.5f, 0.5f },
                // NOTE KI by default just one segement needed
                .segments{ 1, 1, 1 }
            };
        }

        static PrimitiveGenerator height_field()
        {
            return {
                .type = PrimitiveType::height_field,
                .name = "<height_field>",
                .alias = "height_field"
            };
        }

        /// @param size Half of the side length in x (0) and y (1) direction.
        /// @param segments Number of subdivisions in the x (0) and y (1) direction.
        static PrimitiveGenerator plane_grid()
        {
            return {
                .type = PrimitiveType::plane_grid,
                .name = "<plane_grid>",
                .alias = "plane_grid",
                .size{ 0.5f, 0.5f, 0.5f },
                // NOTE KI by default just one segement needed
                .segments{ 1, 1, 1},
            };
        }

        /// @param size Half of the side length in x (0), y (1) and z (2) direction.
        /// @param segments The number of segments in x (0), y (1) and z (2)
        /// directions. All should be >= 1. If any one is zero faces in that
        /// direction are not genereted. If more than one is zero the mesh is empty.
        static PrimitiveGenerator box()
        {
            return {
                .type = PrimitiveType::box,
                .name = "<box>",
                .alias = "box",
                .size{ 0.5f, 0.5f, 0.5f },
                // NOTE KI by default just one segement needed
                .segments{ 1, 1, 1 },
            };
        }

        /// @param radius Radius of the rounded edges.
        /// @param size Half of the side length in x (0), y (1) and z (2) direction.
        /// @param slices Number subdivions around in the rounded edges.
        /// @param segments Number of subdivisons in x (0), y (1) and z (2)
        /// direction for the flat faces.
        static PrimitiveGenerator rounded_box()
        {
            return {
                .type = PrimitiveType::rounded_box,
                .name = "<rounded_box>",
                .alias = "rounded_box",
                .size{ 0.375f, 0.375f, 0.375f },
                .radius = 0.125f,
                .slices = 4,
                .segments = { 8, 8, 8 }
            };
        }

        /// @param radius The radius of the enclosing sphere.
        /// @param segments The number segments along each edge. Should be >= 1.
        /// If <1 empty mesh is generated.
        /// @param rings The number of radial segments on each face. Should be >= 1.
        /// If <1 an empty mesh is generated.
        static PrimitiveGenerator dodeca_hedron()
        {
            return {
                .type = PrimitiveType::dodeca_hedron,
                .name = "<dodeca_hedron>",
                .alias = "dodeca_hedron",
                .radius = 0.5f,
                .segments = { 1, 0, 0 },
                .rings = 1,
            };
        }

        /// @param radius The radius of the enclosing sphere.
        /// @param segments The number segments along each edge. Must be >= 1.
        static PrimitiveGenerator icosa_hedron()
        {
            return {
                .type = PrimitiveType::icosa_hedron,
                .name = "<icosa_hedron>",
                .alias = "icosa_hedron",
                .radius = 0.5f,
                .segments = { 1, 0, 0 },
            };
        }

        /// @param radius The radius of the containing sphere.
        /// @param segments The number of segments per icosahedron edge. Must be >= 1.
        static PrimitiveGenerator ico_sphere()
        {
            return {
                .type = PrimitiveType::ico_sphere,
                .name = "<ico_sphere>",
                .alias = "ico_sphere",
                .radius = 0.5f,
                .segments = { 4, 0, 0 }
            };
        }

        /// @param radius The radius of the sphere
        /// @param slices Subdivisions around the z-azis (longitudes).
        /// @param segments Subdivisions along the z-azis (latitudes).
        static PrimitiveGenerator sphere()
        {
            return {
                .type = PrimitiveType::sphere,
                .name = "<sphere>",
                .alias = "sphere",
                .radius = 0.5f,
                .slices = 32,
                .segments = { 16, 0, 0 }
            };
        }

        /// @param radius Radius of the capsule on the xy-plane.
        /// @param length Half of the length between centers of the caps along the z-axis.
        /// @param slices Number of subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-azis (latitudes).
        /// @param rings Number of radial subdivisions in the caps.
        static PrimitiveGenerator capsule()
        {
            return {
                .type = PrimitiveType::capsule,
                .name = "<capsule>",
                .alias = "capsule",
                .radius = 0.5f,
                .length= 0.5f,
                .slices = 32,
                .segments = { 4, 0, 0 },
                .rings = 8
            };
        }

        /// @param radius Radius of the cylinder along the xy-plane.
        /// @param length Half of the length of the cylinder along the z-axis.
        /// @param slices Subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-axis.
        static PrimitiveGenerator cylinder()
        {
            return {
                .type = PrimitiveType::cylinder,
                .name = "<cylinder>",
                .alias = "cylinder",
                .radius = 0.5f,
                .length= 0.5f,
                .slices = 32,
                // NOTE KI by default just one segement needed
                .segments = { 1, 0, 0 },
            };
        }

        /// @param radius Radius of the cylinder along the xy-plane.
        /// @param length Half of the length of the cylinder along the z-axis.
        /// @param slices Subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-axis.
        static PrimitiveGenerator capped_cylinder()
        {
            return {
                .type = PrimitiveType::capped_cylinder,
                .name = "<capped_cylinder>",
                .alias = "capped_cylinder",
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                // NOTE KI by default just one segement needed
                .segments = { 1, 0, 0 },
            };
        }

        /// @param radius Radius of the cylinder along the xy-plane.
        /// @param length Half of the length of the cylinder along the z-axis.
        /// @param slices Subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-axis.
        static PrimitiveGenerator cone()
        {
            return {
                .type = PrimitiveType::cone,
                .name = "<cone>",
                .alias = "cone",
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                .segments = { 8, 0, 0 },
            };
        }

        /// @param radius Radius of the cylinder along the xy-plane.
        /// @param length Half of the length of the cylinder along the z-axis.
        /// @param slices Subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-axis.
        static PrimitiveGenerator capped_cone()
        {
            return {
                .type = PrimitiveType::capped_cone,
                .name = "<capped_cone>",
                .alias = "capped_cone",
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                .segments = { 8, 0, 0 },
            };
        }

        /// @param radius Radius of the negative z end on the xy-plane.
        /// @param size Half of the distance between cap and tip along the z-axis.
        /// @param slices Number of subdivisions around the z-axis.
        /// @param segments Number subdivisions along the z-axis.
        /// @param rings Number subdivisions in the cap.
        /// @param start Counterclockwise angle around the z-axis relative to the positive x-axis.
        /// @param sweep Counterclockwise angle around the z-axis.
        static PrimitiveGenerator spherical_cone()
        {
            return {
                .type = PrimitiveType::spherical_cone,
                .name = "<capped_cone>",
                .alias = "capped_cone",
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                .segments = { 8, 0, 0 },
                .rings = 4
            };
        }

        /// @param radius The outer radius of the cylinder on the xy-plane.
        /// @param innerRadius The inner radius of the cylinder on the xy-plane.
        /// @param size Half of the length of the cylinder along the z-axis.
        /// @param slices Subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-axis.
        /// @param start Counterclockwise angle around the z-axis relative to the x-axis.
        /// @param sweep Counterclockwise angle around the z-axis.
        static PrimitiveGenerator tube()
        {
            return {
                .type = PrimitiveType::tube,
                .name = "<tube>",
                .alias = "tube",
                .inner_radius = 0.3f,
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                // NOTE KI by default just one segement needed
                .segments = { 1, 0, 0 },
            };
        }

        /// @param radius The outer radius of the cylinder on the xy-plane.
        /// @param innerRadius The inner radius of the cylinder on the xy-plane.
        /// @param size Half of the length of the cylinder along the z-axis.
        /// @param slices Number nubdivisions around the z-axis.
        /// @param segments Number of subdivisions along the z-axis.
        /// @param rings Number radial subdivisions in the cap.
        /// @param start Counterclockwise angle around the z-axis relative to the x-axis.
        /// @param sweep Counterclockwise angle around the z-axis.
        static PrimitiveGenerator capped_tube()
        {
            return {
                .type = PrimitiveType::capped_tube,
                .name = "<capped_tube>",
                .alias = "capped_tube",
                .inner_radius = 0.3f,
                .radius = 0.5f,
                .length = 0.5f,
                .slices = 32,
                // NOTE KI by default just one segement needed
                .segments = { 1, 0, 0 },
                .rings = 1,
            };
        }

        /// @param minor Radius of the minor (inner) ring
        /// @param major Radius of the major (outer) ring
        /// @param slices Subdivisions around the minor ring
        /// @param segments Subdivisions around the major ring
        /// @param minorStart Counterclockwise angle relative to the xy-plane.
        /// @param minorSweep Counterclockwise angle around the circle.
        /// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
        /// @param majorSweep Counterclockwise angle around the z-axis.
        static PrimitiveGenerator torus()
        {
            return {
                .type = PrimitiveType::torus,
                .name = "<torus>",
                .alias = "torus",
                .inner_radius = 0.125f,
                .radius = 0.375f,
                .slices = 16,
                .segments = { 32, 0, 0 },
            };
        }

        /// @param slices Number subdivisions around the circle.
        /// @param segments Number of subdivisions around the path.
        static PrimitiveGenerator torus_knot()
        {
            return {
                .type = PrimitiveType::torus_knot,
                .name = "<torus_knot>",
                .alias = "torus_knot",
                .p = 2,
                .q = 3,
                .slices = 8,
                .segments = { 96, 0, 0 },
            };
        }

        /// @param radius Outer radius of the disk on the xy-plane.
        /// @param innerRadius radius of the inner circle on the xy-plane.
        /// @param slices Number of subdivisions around the z-axis.
        /// @param rings Number of subdivisions along the radius.
        /// @param start Counterclockwise angle relative to the x-axis
        /// @param sweep Counterclockwise angle.
        static PrimitiveGenerator disk()
        {
            return {
                .type = PrimitiveType::disk,
                .name = "<disk>",
                .alias = "disk",
                .inner_radius = 0.f,
                .radius = 0.5f,
                .slices = 32,
                .rings = 4,
            };
        }

        /// @param minor Radius of the spring it self.
        /// @param major Radius from the z-axis
        /// @param size Half of the length along the z-axis.
        /// @param slices Subdivisions around the spring.
        /// @param segments Subdivisions along the path.
        /// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
        /// @param majorSweep Counterclockwise angle arounf the z-axis.
        static PrimitiveGenerator spring()
        {
            return {
                .type = PrimitiveType::spring,
                .name = "<spring>",
                .alias = "spring",
                .inner_radius = 0.125f,
                .radius = 0.375f,
                .length = 0.5f,
                .slices = 8,
                .segments = { 32, 0, 0 },
            };
        }

        static PrimitiveGenerator get(PrimitiveType type)
        {
            switch (type) {
            case PrimitiveType::points:
                return points();
            case PrimitiveType::lines:
                return lines();
            case PrimitiveType::ray:
                return ray();
            case PrimitiveType::bezier:
                return bezier();
            case PrimitiveType::plane:
                return plane();
            case PrimitiveType::quad:
                return quad();
            case PrimitiveType::height_field:
                return height_field();
            case PrimitiveType::plane_grid:
                return plane_grid();
            case PrimitiveType::box:
                return box();
            case PrimitiveType::rounded_box:
                return rounded_box();
            case PrimitiveType::dodeca_hedron:
                return dodeca_hedron();
            case PrimitiveType::icosa_hedron:
                return icosa_hedron();
            case PrimitiveType::ico_sphere:
                return ico_sphere();
            case PrimitiveType::sphere:
                return sphere();
            case PrimitiveType::capsule:
                return capsule();
            case PrimitiveType::cylinder:
                return cylinder();
            case PrimitiveType::capped_cylinder:
                return capped_cylinder();
            case PrimitiveType::cone:
                return cone();
            case PrimitiveType::capped_cone:
                return capped_cone();
            case PrimitiveType::spherical_cone:
                return spherical_cone();
            case PrimitiveType::tube:
                return tube();
            case PrimitiveType::capped_tube:
                return capped_tube();
            case PrimitiveType::torus:
                return torus();
            case PrimitiveType::torus_knot:
                return torus_knot();
            case PrimitiveType::disk:
                return disk();
            case PrimitiveType::spring:
                return spring();
            }

            return none();
        }

        std::shared_ptr<mesh::Mesh> create() const;
    };
}
