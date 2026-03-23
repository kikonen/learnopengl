#include "LodMeshContainer.h"

#include "LodMesh.h"
#include "MeshSet.h"

namespace mesh
{
    LodMeshContainer::LodMeshContainer() = default;
    LodMeshContainer::~LodMeshContainer() = default;

    uint16_t LodMeshContainer::addMeshSet(
        const mesh::MeshSet& meshSet) noexcept
    {
        uint16_t count = 0;

        for (auto& mesh : meshSet.getMeshes()) {
            auto* lodMesh = addLodMesh({ mesh });
            count++;
        }

        return count;
    }

    mesh::LodMesh* LodMeshContainer::addLodMesh(
        mesh::LodMesh&& lodmesh) noexcept
    {
        m_lodMeshes.push_back(std::move(lodmesh));
        return &m_lodMeshes[m_lodMeshes.size() - 1];
    }
}
