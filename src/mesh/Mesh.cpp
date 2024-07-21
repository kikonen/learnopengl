#include "Mesh.h"

#include <fmt/format.h>

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<ki::mesh_id> ID_GENERATOR;
}

namespace mesh {
    Mesh::Mesh(std::string_view name)
        : m_id{ ID_GENERATOR.nextId() },
        m_name{ name }
    {
    }

    Mesh::~Mesh()
    {
        KI_INFO(fmt::format("MESH: delete {}", str()));
    }

    std::string Mesh::str() const noexcept
    {
        return fmt::format(
            "<MESH: id={}, name={}, indeces={}, vertices={}>",
            m_id,
            m_name,
            m_indeces.size(),
            m_vertices.size());
    }

    AABB Mesh::calculateAABB(const glm::mat4& transform) const
    {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = transform * glm::vec4(vertex.pos, 1.f);
            aabb.minmax(pos);
        }

        aabb.updateVolume();

        return aabb;
    }
}
