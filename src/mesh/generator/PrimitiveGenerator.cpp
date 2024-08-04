#include "PrimitiveGenerator.h"

#include <glm/glm.hpp>
#include <fmt/format.h>
#include <generator/generator.hpp>

#include "util/Util.h"
#include "util/Log.h"
#include "util/glm_util.h"
#include "util/glm_format.h"

#include "mesh/PrimitiveMesh.h"

namespace {
    // NOTE KI normal, tangent, tex stored to allow normal g_tex shader
    const float QUAD_VERTICES[] = {
        // pos              // normal         // tangent        // tex
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    // NOTE KI normal, tangent, tex stored to allow normal g_tex shader
    const float PLANE_VERTICES[] = {
        // pos              // normal         // tangent        // tex
        -1.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         1.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    };

    const int QUAD_INDECES[] = {
        0, 1, 2,
        2, 1, 3
    };

    std::unique_ptr<mesh::Mesh> createVertices(
        mesh::PrimitiveGenerator generator)
    {
        if (generator.vertices.empty() || generator.indeces.empty()) return nullptr;

        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(generator.vertices.size());
        indeces.reserve(generator.indeces.size());

        for (auto& v : generator.vertices) {
            auto& vertex = vertices.emplace_back();
            vertex.pos = v;
        }

        for (auto& v : generator.indeces) {
            indeces.push_back(v);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> createRay(
        mesh::PrimitiveGenerator generator)
    {
        if (generator.length == 0) return nullptr;

        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(2);
        indeces.reserve(2);

        const auto& origin = generator.origin;
        const auto& dir = generator.dir;
        const auto& length = generator.length;

        {
            auto& v = vertices.emplace_back();
            v.pos = origin;
            v.normal = { 0, 1, 0 };
            v.tangent = { 0, 0, 1 };
        }
        {
            auto& v = vertices.emplace_back();
            v.pos = origin + dir * length;
            v.normal = { 0, 1, 0 };
            v.tangent = { 0, 0, 1 };
        }

        indeces.push_back(0);
        indeces.push_back(1);

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> createPlane(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(4);
        indeces.reserve(indeces.size());

        glm::vec2 size{ generator.size.x, generator.size.y };
        glm::mat4 scaleMat = glm::scale(glm::mat4{ 1.f }, glm::vec3{ size.x, 1.f, size.y });

        auto& row = PLANE_VERTICES;
        for (int i = 0; i < 4; i++) {
            auto& v = vertices.emplace_back();

            int offset = 11 * i;
            v.pos = scaleMat * glm::vec4{ row[offset + 0], row[offset + 1] , row[offset + 2], 1.f };

            offset += 3;
            v.normal = glm::vec3{ row[offset + 0], row[offset + 1], row[offset + 2] };

            offset += 3;
            v.tangent = glm::vec3{ row[offset + 0], row[offset + 1] , row[offset + 2] };

            offset += 3;
            v.texCoord = glm::vec2{ row[offset + 0], row[offset + 1] };
        }

        for (auto& index : QUAD_INDECES) {
            indeces.push_back(index);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> createQuad(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(4);
        indeces.reserve(indeces.size());

        glm::vec2 size{ generator.size.x, generator.size.y };
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
            v.texCoord = glm::vec2{ row[offset + 0], row[offset + 1] };
        }

        for (auto& index : QUAD_INDECES) {
            indeces.push_back(index);
        }

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> createBox(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::BoxMesh shape{ generator.size, { 1, 1, 1 } };

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

    std::unique_ptr<mesh::Mesh> createSphere(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SphereMesh shape{
            generator.radius,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createCapsule(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CapsuleMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
            generator.rings
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

    std::unique_ptr<mesh::Mesh> createCylinder(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CylinderMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createCappedCylinder(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CappedCylinderMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createCone(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::ConeMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createCappedCone(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CappedConeMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createTorus(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::TorusMesh shape{
            generator.inner_radius,
            generator.radius,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createDisk(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::DiskMesh shape{
            generator.inner_radius,
            generator.radius,
            generator.slices,
            generator.segments,
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

    std::unique_ptr<mesh::Mesh> createSpring(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SpringMesh shape{
            generator.inner_radius,
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments,
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

namespace mesh {
    std::unique_ptr<mesh::Mesh> PrimitiveGenerator::create() const
    {
        switch (type) {
        case PrimitiveType::points:
            return createVertices(*this);
        case PrimitiveType::lines:
            return createVertices(*this);
        case PrimitiveType::ray:
            return createRay(*this);
        case PrimitiveType::plane:
            return createPlane(*this);
        case PrimitiveType::quad:
            return createQuad(*this);
        case PrimitiveType::box:
            return createBox(*this);
        case PrimitiveType::sphere:
            return createSphere(*this);
        case PrimitiveType::capsule:
            return createCapsule(*this);
        case PrimitiveType::cylinder:
            return createCylinder(*this);
        case PrimitiveType::capped_cylinder:
            return createCappedCylinder(*this);
        case PrimitiveType::cone:
            return createCone(*this);
        case PrimitiveType::capped_cone:
            return createCappedCone(*this);
        case PrimitiveType::torus:
            return createTorus(*this);
        case PrimitiveType::disk:
            return createDisk(*this);
        case PrimitiveType::spring:
            return createSpring(*this);
        }

        return nullptr;
    }
}
