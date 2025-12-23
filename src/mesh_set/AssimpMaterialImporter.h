#pragma once

#include <map>

#include "material/Material.h"

#include "MeshSetImporter.h"

namespace mesh
{
    class MeshSet;
}

struct aiScene;
struct aiMaterial;

namespace mesh_set
{
    class AssimpMaterialImporter
    {
    public:
        AssimpMaterialImporter(
            bool debug);
        ~AssimpMaterialImporter();

        void processMaterials(
            const mesh::MeshSet& meshSet,
            std::vector<Material>& materials,
            std::map<size_t, size_t>& materialMapping,
            const aiScene* scene);

        Material processMaterial(
            const mesh::MeshSet& meshSet,
            const aiScene* scene,
            const aiMaterial* material);

        std::string findTexturePath(
            const mesh::MeshSet& meshSet,
            const std::string& origPath);

    private:
        const bool m_debug;
    };
}
