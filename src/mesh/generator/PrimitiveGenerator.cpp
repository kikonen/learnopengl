#include "PrimitiveGenerator.h"

#include <glm/glm.hpp>
#include <fmt/format.h>
#include <generator/generator.hpp>

#include "util/Util.h"
#include "util/Log.h"
#include "util/glm_format.h"

#include "mesh/ModelMesh.h"

namespace mesh {
    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateSphere(
        std::string_view name,
        int slices,
        int segments) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SphereMesh shape{
            1.0f,
            slices,
            segments,
        };

        for (const auto& vertex : shape.vertices()) {
            int x = 0;
            KI_INFO_OUT(fmt::format("SPHERE: VER: pos={}, uv={}, normal={}", vertex.position, vertex.texCoord, vertex.normal));
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            int x = 0;
            KI_INFO_OUT(fmt::format("SPHERE: IND: vertices={}", tri.vertices));
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateCapsule(
        std::string_view name,
        float size,
        int slices,
        int segments,
        int rings) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CapsuleMesh shape{
            1.0f,
            size,
            slices,
            segments,
        };

        for (const auto& vertex : shape.vertices()) {
            int x = 0;
            KI_INFO_OUT(fmt::format("CAPSULE: VER: pos={}, uv={}, normal={}", vertex.position, vertex.texCoord, vertex.normal));
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            int x = 0;
            KI_INFO_OUT(fmt::format("CAPSULE: IND: vertices={}", tri.vertices));
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }
}
