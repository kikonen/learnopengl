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
struct aiAnimation;
struct aiMaterial;

namespace animation {
    struct AnimationContainer;
    struct AnimationNode;
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
        void processSkeleton(
            animation::AnimationContainer& animContainer,
            ModelMesh& mesh,
            size_t skeletonIndex,
            const aiScene* scene,
            const aiSkeleton* skeleton);

        void processSkeletonBone(
            animation::AnimationContainer& animContainer,
            ModelMesh& mesh,
            size_t skeletonIndex,
            size_t boneIndex,
            const aiScene* scene,
            const aiSkeleton* skeleton,
            const aiSkeletonBone* bone);

        void processAnimation(
            animation::AnimationContainer& animContainer,
            ModelMesh& mesh,
            size_t animIndex,
            const aiScene* scene,
            const aiAnimation* animation);

        void collectNodes(
            animation::AnimationContainer& animContainer,
            const aiScene* scene,
            const aiNode* node,
            int16_t parentId,
            const glm::mat4& parentTransform);

        void processMeshes(
            animation::AnimationContainer& animContainer,
            ModelMesh& mesh,
            const aiScene* scene);

        void processMesh(
            animation::AnimationContainer& animContainer,
            animation::AnimationNode& animNode,
            ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh);

        void processMeshFace(
            animation::AnimationContainer& animContainer,
            animation::AnimationNode& animNode,
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t faceIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiFace* face);

        void processMeshBone(
            animation::AnimationContainer& animContainer,
            animation::AnimationNode& animNode,
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t boneIndex,
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
