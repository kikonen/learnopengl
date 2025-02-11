#include "Mesh.h"

#include "util/debug.h"

#include <fmt/format.h>

#include "pool/IdGenerator.h"

#include "registry/VaoRegistry.h"

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
        //KI_INFO(fmt::format("MESH: delete {}", str()));
    }

    std::string Mesh::str() const noexcept
    {
        return fmt::format(
            "<MESH: id={}, name={}, alias={}, baseVertex={}, baseIndex={}, vertexCount={}, indexCount={}>",
            m_id,
            m_name,
            m_alias,
            getBaseVertex(),
            getBaseIndex(),
            getDefinedVertexCount(),
            getDefinedIndexCount());
    }

    const kigl::GLVertexArray* Mesh::prepareVAO()
    {
        return setupVAO(VaoRegistry::get().getTexturedVao(), true);
    }
}
