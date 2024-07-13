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
        return fmt::format("<MESH: id={}>", m_id);
    }
}
