#include "MeshInfo.h"

#include <assimp/scene.h>

#include "mesh/ModelMesh.h"

namespace animation {
    MeshInfo::MeshInfo(const mesh::ModelMesh* mesh)
        : m_name{ mesh->m_name },
        m_material{ mesh->getMaterial() ? mesh->getMaterial()->m_name : "NULL"},
        m_vertexCount{ static_cast<uint32_t>(mesh->m_vertices.size()) },
        m_indexCount{ static_cast<uint32_t>(mesh->m_indeces.size()) }
    {
    }
}
