#pragma once

#include <string>
#include <vector>
#include <memory>

namespace animation
{
    struct RigContainer;
}

namespace mesh {
    class Mesh;
    struct LodMesh;

    mesh::LodMesh* findLodMesh(
        const std::string& meshName,
        std::vector<mesh::LodMesh>& lodMeshes);

    std::shared_ptr<animation::RigContainer> findRig(
        std::vector<mesh::LodMesh>& lodMeshes);
}
