#include "Mesh.h"

#include <fmt/format.h>

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<ki::mesh_id> ID_GENERATOR;
}

namespace mesh {
    Mesh::Mesh()
        : m_id(ID_GENERATOR.nextId())
    {
    }

    Mesh::~Mesh()
    {
        KI_INFO(fmt::format("MESH: delete {}", str()));
    }

    const std::string Mesh::str() const noexcept
    {
        return fmt::format("<MESH: id={}>", m_id);
    }

    void Mesh::prepareVolume() {
        setAABB(calculateAABB());
    }
}
