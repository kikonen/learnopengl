#pragma once

#include <map>
#include <memory>
#include <atomic>

#include "asset/Material.h"

#include "mesh/ModelLoader.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiFace;
struct aiMaterial;

namespace mesh {
    class AssimpLoader : public ModelLoader {
    public:
        AssimpLoader(
            std::shared_ptr<std::atomic<bool>> alive);

        ~AssimpLoader();

    protected:
        void loadData(
            ModelMesh& mesh);

        void loadResolvedPath(
            ModelMesh& modelMesh);

    private:
        void processNode(
            ModelMesh& mesh,
            const std::map<size_t, ki::material_id>& materialMapping,
            const aiScene* scene,
            const aiNode* node);

        void processMesh(
            ModelMesh& modelMesh,
            const std::map<size_t, ki::material_id>& materialMapping,
            size_t meshIndex,
            const aiNode* node,
            const aiMesh* mesh);

        void processFace(
            ModelMesh& modelMesh,
            const std::map<size_t, ki::material_id>& materialMapping,
            size_t faceIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiFace* face);

        void processMaterials(
            ModelMesh& modelMesh,
            std::map<size_t, ki::material_id>& materialMapping,
            const aiScene* scene);

        Material processMaterial(
            ModelMesh& modelMesh,
            const aiScene* scene,
            const aiMaterial* material);

        std::string findTexturePath(
            ModelMesh& modelMesh,
            std::string assetPath);
    };
}
