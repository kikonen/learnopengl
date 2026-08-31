#include "MeshInfo.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

#include "animation/Rig.h"

#include "mesh/ModelMesh.h"

namespace animation {
    MeshInfo::MeshInfo(const mesh::ModelMesh* mesh)
        : m_name{ util::assimp::normalizeName(mesh->getName()) },
        m_material{ mesh->getMaterial() ? mesh->getMaterial()->m_name : "NULL"},
        m_rig{ mesh->getRig() ? mesh->getRig()->getName() : "NULL"},
        m_vertexCount{ static_cast<uint32_t>(mesh->getVertices().size())},
        m_indexCount{ static_cast<uint32_t>(mesh->getIndeces().size())}
    {
    }
}
