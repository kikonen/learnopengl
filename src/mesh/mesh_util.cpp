#include "mesh_util.h"

#include "LodMesh.h"
#include "ModelMesh.h"

namespace mesh{
    mesh::LodMesh* findLodMesh(
        const std::string& meshName,
        std::vector<mesh::LodMesh>& lodMeshes)
    {
        const auto& it = std::find_if(
            lodMeshes.begin(),
            lodMeshes.end(),
            [&meshName](const auto& lodMesh) {
                return meshName == lodMesh.m_mesh->m_name ||
                    meshName == lodMesh.m_mesh->m_alias;
            });
        if (it == lodMeshes.end()) return nullptr;
        return &(*it);
    }

    std::shared_ptr<animation::RigContainer> findRig(
        std::vector<mesh::LodMesh>& lodMeshes)
    {
        const auto& it = std::find_if(
            lodMeshes.begin(),
            lodMeshes.end(),
            [](const auto& lodMesh) {
                const auto* mesh = lodMesh.getMesh<mesh::ModelMesh>();
                return mesh && mesh->m_rig;
            });
        if (it == lodMeshes.end()) return nullptr;


        const auto* mesh = it->getMesh<mesh::ModelMesh>();
        return mesh->m_rig;
    }
}
