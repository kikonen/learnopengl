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
                return lodMesh.m_mesh->match(meshName);
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

    std::vector<std::string> getLodMeshNames(
        const std::string& meshName,
        std::vector<mesh::LodMesh>& lodMeshes)
    {
        std::vector<std::string> names;

        for (auto& lodMesh : lodMeshes) {
            names.push_back(lodMesh.m_mesh->m_name);
        }
        return names;
    }

    std::vector<std::string> getLodMeshAliases(
        const std::string& meshName,
        std::vector<mesh::LodMesh>& lodMeshes)
    {
        std::vector<std::string> aliases;

        for (auto& lodMesh : lodMeshes) {
            aliases.push_back(lodMesh.m_mesh->m_alias);
        }

        return aliases;
    }

}
