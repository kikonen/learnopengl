#pragma once

#include <vector>

#include "LodMesh.h"

namespace mesh
{
    class MeshSet;
    struct LodMesh;

    class LodMeshContainer
    {
    public:
        LodMeshContainer();
        ~LodMeshContainer();

        // @return count of meshes added
        uint16_t addMeshSet(
            const mesh::MeshSet& meshSet) noexcept;

        size_t size() const
        {
            return m_lodMeshes.size();
        }

        mesh::LodMesh* addLodMesh(mesh::LodMesh&& lodMesh) noexcept;

        inline const mesh::LodMesh* getLodMesh(uint8_t lodIndex) const noexcept
        {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline const std::vector<mesh::LodMesh>& getLodMeshes() const noexcept
        {
            return m_lodMeshes;
        }

        inline mesh::LodMesh* modifyLodMesh(uint8_t lodIndex) noexcept {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline std::vector<mesh::LodMesh>& modifyLodMeshes() noexcept {
            return m_lodMeshes;
        }

        inline bool hasMeshes() const noexcept
        {
            return !m_lodMeshes.empty() && m_lodMeshes[0].m_mesh.get();
        }

    private:
        std::vector<mesh::LodMesh> m_lodMeshes;
    };
}
