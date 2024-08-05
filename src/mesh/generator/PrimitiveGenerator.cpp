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

    std::unique_ptr<mesh::Mesh> create_ray(
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

    std::unique_ptr<mesh::Mesh> create_plane(
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
            //v.pos *= 0.5;

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

    std::unique_ptr<mesh::Mesh> create_quad(
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
            //v.pos *= 0.5;

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

    std::unique_ptr<mesh::Mesh> create_box(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::BoxMesh shape{
            generator.size,
            { 1, 1, 1 } };

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

    std::unique_ptr<mesh::Mesh> create_rounded_box(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::RoundedBoxMesh shape{
            generator.radius,
            generator.size,
            generator.slices,
            generator.segments };

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

    std::unique_ptr<mesh::Mesh> create_sphere(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_capsule(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_cylinder(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_capped_cylinder(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_cone(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_capped_cone(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SphericalConeMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_spherical_cone(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::SphericalConeMesh shape{
            generator.radius,
            generator.length,
            generator.slices,
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_tube(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::TubeMesh shape{
            generator.radius,
            generator.inner_radius,
            generator.length,
            generator.slices,
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_capped_tube(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::CappedTubeMesh shape{
            generator.radius,
            generator.inner_radius,
            generator.length,
            generator.slices,
            generator.segments.x,
            generator.rings,
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

    std::unique_ptr<mesh::Mesh> create_torus(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_torus_knot(
        mesh::PrimitiveGenerator generator)
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(generator.name);
        mesh->m_type = generator.type;
        mesh->m_alias = generator.alias;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        generator::TorusKnotMesh shape{
            generator.p,
            generator.q,
            generator.slices,
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_disk(
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
            generator.segments.x,
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

    std::unique_ptr<mesh::Mesh> create_spring(
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
            generator.segments.x,
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
            return create_ray(*this);
        case PrimitiveType::plane:
            return create_plane(*this);
        case PrimitiveType::quad:
            return create_quad(*this);
        case PrimitiveType::box:
            return create_box(*this);
        case PrimitiveType::rounded_box:
            return create_rounded_box(*this);
        case PrimitiveType::sphere:
            return create_sphere(*this);
        case PrimitiveType::capsule:
            return create_capsule(*this);
        case PrimitiveType::cylinder:
            return create_cylinder(*this);
        case PrimitiveType::capped_cylinder:
            return create_capped_cylinder(*this);
        case PrimitiveType::cone:
            return create_cone(*this);
        case PrimitiveType::capped_cone:
            return create_capped_cone(*this);
        case PrimitiveType::spherical_cone:
            return create_spherical_cone(*this);
        case PrimitiveType::tube:
            return create_tube(*this);
        case PrimitiveType::capped_tube:
            return create_capped_tube(*this);
        case PrimitiveType::torus:
            return create_torus(*this);
        case PrimitiveType::torus_knot:
            return create_torus_knot(*this);
        case PrimitiveType::disk:
            return create_disk(*this);
        case PrimitiveType::spring:
            return create_spring(*this);
        }

        return nullptr;
    }
}
