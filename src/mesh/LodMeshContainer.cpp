#include "LodMeshContainer.h"

#include "LodMesh.h"
#include "MeshSet.h"

namespace mesh
{
    LodMeshContainer::LodMeshContainer() = default;
    LodMeshContainer::~LodMeshContainer() = default;

    uint16_t LodMeshContainer::addMeshSet(
        const mesh::MeshSet& meshSet,
        ki::sid_t lodMeshId) noexcept
    {
        uint16_t count = 0;

        for (auto& mesh : meshSet.getMeshes()) {
            auto* lodMesh = addLodMesh({ mesh }, lodMeshId);
            count++;
        }

        return count;
    }

    mesh::LodMesh* LodMeshContainer::addLodMesh(
        mesh::LodMesh&& lodmesh,
        ki::sid_t lodMeshId) noexcept
    {
        m_lodMeshes.push_back(util::Ref<mesh::LodMesh>{ new mesh::LodMesh{ std::move(lodmesh) } });
        auto* lodMesh = m_lodMeshes.back().get();
        lodMesh->m_id = lodMeshId;
        return lodMesh;
    }
}
