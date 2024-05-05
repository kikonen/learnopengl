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
struct aiBone;
struct aiSkeleton;
struct aiSkeletonBone;
struct aiMaterial;

namespace animation {
    struct RigContainer;
    struct RigNode;
}

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
        void collectNodes(
            animation::RigContainer& rig,
            std::vector<const aiNode*>& assimpNodes,
            const aiScene* scene,
            const aiNode* node,
            int16_t parentIndex,
            const glm::mat4& parentTransform);

        void loadAnimations(
            animation::RigContainer& rig,
            const std::string& namePrefix,
            const aiScene* scene);

        void processMeshes(
            animation::RigContainer& rig,
            const std::vector<const aiNode*>& assimpNodes,
            ModelMesh& mesh,
            const aiScene* scene);

        void processMesh(
            animation::RigContainer& rig,
            animation::RigNode& rigNode,
            ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh);

        void processMeshFace(
            animation::RigContainer& rig,
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t faceIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiFace* face);

        void processMeshBone(
            animation::RigContainer& rig,
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiBone* bone);

        void processMaterials(
            std::map<size_t, ki::material_id>& materialMapping,
            ModelMesh& modelMesh,
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
