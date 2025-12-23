#pragma once

#include <string>
#include <vector>
#include <memory>

namespace animation
{
    struct Rig;
}

namespace mesh {
    class Mesh;
    struct LodMesh;

    mesh::LodMesh* findLodMesh(
        const std::string& meshName,
        std::vector<mesh::LodMesh>& lodMeshes);

    std::vector<std::string> getLodMeshNames(
        const std::string& meshName,
        const std::vector<mesh::LodMesh>& lodMeshes);

    std::vector<std::string> getLodMeshAliases(
        const std::string& meshName,
        const std::vector<mesh::LodMesh>& lodMeshes);
}
