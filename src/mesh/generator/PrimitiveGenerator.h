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
        float radius = 1.f;
        float length{ 0.5f };
        int slices{ 32 };
        int segments{ 4 };
        int rings{ 8 };

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

        /// @param size Half of the length between centers of the caps along the z-axis.
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
        /// @param size Half of the length of the cylinder along the z-axis.
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
        /// @param size Half of the length of the cylinder along the z-axis.
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

        static PrimitiveGenerator get(PrimitiveType type)
        {
            switch (type) {
            case PrimitiveType::points:
                return points();
            case PrimitiveType::lines:
                return lines();
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
            }

            return none();
        }

        std::unique_ptr<mesh::Mesh> create() const;
    };
}
