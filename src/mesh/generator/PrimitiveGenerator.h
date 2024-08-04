#pragma once

#include <string_view>
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

        glm::vec3 size{ 1.f, 1.f, 1.f };
        float inner_radius = 0.f;
        float radius = 1.f;
        float length{ 0.5f };
        int slices{ 32 };
        int segments{ 4 };
        int rings{ 8 };

        glm::vec3 origin{ 0.f };
        glm::vec3 dir{ 0.f };

        std::vector<glm::vec3> vertices;
        std::vector<int> indeces;

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

        static PrimitiveGenerator plane()
        {
            return {
                .type = PrimitiveType::plane,
                .name = "<plane>",
                .alias = "plane"
            };
        }

        static PrimitiveGenerator quad()
        {
            return {
                .type = PrimitiveType::quad,
                .name = "<quad>",
                .alias = "quad"
            };
        }

        static PrimitiveGenerator box()
        {
            return {
                .type = PrimitiveType::box,
                .name = "<box>",
                .alias = "box"
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
                .radius = 1.f,
                .slices = 32,
                .segments = 16
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
                .radius = 1.f,
                .length= 0.5f,
                .slices = 32,
                .segments = 4,
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
                .radius = 1.f,
                .length= 1.f,
                .slices = 32,
                .segments = 8,
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
                .radius = 1.f,
                .length = 1.f,
                .slices = 32,
                .segments = 8,
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
                .radius = 1.f,
                .length = 1.f,
                .slices = 32,
                .segments = 8,
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
                .radius = 1.f,
                .length = 1.f,
                .slices = 32,
                .segments = 8,
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
                .inner_radius = 0.25f,
                .radius = 1.f,
                .slices = 16,
                .segments = 32,
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
                .radius = 1.f,
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
                .inner_radius = 0.25f,
                .radius = 1.f,
                .length = 1.f,
                .slices = 8,
                .segments = 32,
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
            case PrimitiveType::plane:
                return plane();
            case PrimitiveType::quad:
                return quad();
            case PrimitiveType::box:
                return box();
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
            case PrimitiveType::torus:
                return torus();
            case PrimitiveType::disk:
                return disk();
            case PrimitiveType::spring:
                return spring();
            }

            return none();
        }

        std::unique_ptr<mesh::Mesh> create() const;
    };
}
