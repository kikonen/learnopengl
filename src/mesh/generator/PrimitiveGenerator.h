#pragma once

#include <string_view>
#include <memory>

namespace mesh {
    class Mesh;

    struct PrimitiveGenerator {
        std::unique_ptr<mesh::Mesh> generateQuad(
            std::string_view name) const;

        /// @param slices Subdivisions around the z-azis (longitudes).
        /// @param segments Subdivisions along the z-azis (latitudes).
        std::unique_ptr<mesh::Mesh> generateSphere(
            std::string_view name,
            int slices = 32,
            int segments = 16) const;

        /// @param size Half of the length between centers of the caps along the z-axis.
        /// @param slices Number of subdivisions around the z-axis.
        /// @param segments Subdivisions along the z-azis (latitudes).
        /// @param rings Number of radial subdivisions in the caps.
        std::unique_ptr<mesh::Mesh> generateCapsule(
            std::string_view name,
            float size = 0.5f,
            int slices = 32,
            int segments = 4,
            int rings = 8) const;
    };
}
