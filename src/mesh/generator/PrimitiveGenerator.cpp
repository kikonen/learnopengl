#include "PrimitiveGenerator.h"

#include <glm/glm.hpp>
#include <fmt/format.h>
#include <generator/generator.hpp>

#include "util/Util.h"
#include "util/Log.h"
#include "util/glm_util.h"
#include "util/glm_format.h"

#include "mesh/ModelMesh.h"

namespace {
    // NOTE KI normal, tangent, tex stored to allow normal g_tex shader
    const float QUAD_VERTICES[] = {
        // pos              // normal         // tangent        // tex
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    const int QUAD_INDECES[] = {
        0, 1, 2,
        2, 1, 3
    };
}

namespace mesh {
    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generatePlane(
        std::string_view name,
        const glm::vec2& size) const
    {
        auto mesh = generateQuad(name, size);

        glm::mat3 rotateMat = glm::mat4(util::degreesToQuat({ -90, 0, 0 }));

        for (auto& vertex : mesh->m_vertices) {
            vertex.pos = rotateMat * vertex.pos;
            vertex.normal = rotateMat * vertex.normal;
        }

        return std::move(mesh);
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateQuad(
        std::string_view name,
        const glm::vec2& size) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(4);
        indeces.reserve(indeces.size());

        glm::mat4 scaleMat = glm::scale(glm::mat4{ 1.f }, glm::vec3{ size, 1.f });

        auto& row = QUAD_VERTICES;
        for (int i = 0; i < 4; i++) {
            auto& v = vertices.emplace_back();

            int offset = 11 * i;
            v.pos = scaleMat * glm::vec4{ row[offset + 0], row[offset + 1] , row[offset + 2], 1.f };

            offset += 3;
            v.normal = glm::vec3{ row[offset + 0], row[offset + 1], row[offset + 2] };

            offset += 3;
            v.tangent = glm::vec3{ row[offset + 0], row[offset + 1] , row[offset + 2] };

            offset += 3;
            v.texCoord= glm::vec2{ row[offset + 0], row[offset + 1] };
        }

        for (auto& index : QUAD_INDECES) {
            indeces.push_back(index);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateBox(
        std::string_view name,
        const glm::vec3& size) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::BoxMesh shape{ size, {1, 1, 1} };

        for (const auto& vertex : shape.vertices()) {
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateSphere(
        std::string_view name,
        float radius,
        int slices,
        int segments) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SphereMesh shape{
            radius,
            slices,
            segments,
        };

        for (const auto& vertex : shape.vertices()) {
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateCapsule(
        std::string_view name,
        float radius,
        float size,
        int slices,
        int segments,
        int rings) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CapsuleMesh shape{
            radius,
            size,
            slices,
            segments,
            rings
        };

        for (const auto& vertex : shape.vertices()) {
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::generateCylinder(
        std::string_view name,
        float radius,
        float size,
        int slices,
        int segments) const
    {
        auto mesh = std::make_unique<mesh::ModelMesh>(name);

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CylinderMesh shape{
            radius,
            size,
            slices,
            segments,
        };

        for (const auto& vertex : shape.vertices()) {
            auto& v = vertices.emplace_back(
                vertex.position,
                vertex.texCoord,
                vertex.normal,
                vertex.normal);
        }

        for (const auto& tri : shape.triangles()) {
            indeces.push_back(tri.vertices[0]);
            indeces.push_back(tri.vertices[1]);
            indeces.push_back(tri.vertices[2]);
        }

        return mesh;
    }
}
