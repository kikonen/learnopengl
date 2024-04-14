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
            ModelMesh& mesh,
            size_t skeletonIndex,
            const aiScene* scene,
            const aiSkeleton* skeleton);

        void processSkeletonBone(
            ModelMesh& mesh,
            size_t skeletonIndex,
            size_t boneIndex,
            const aiScene* scene,
            const aiSkeleton* skeleton,
            const aiSkeletonBone* bone);

        void processAnimation(
            ModelMesh& mesh,
            size_t animIndex,
            const aiScene* scene,
            const aiAnimation* animation);

        void processNode(
            ModelMesh& mesh,
            const std::map<size_t, ki::material_id>& materialMapping,
            const aiScene* scene,
            const aiNode* node,
            int nodeLevel,
            const glm::mat4& parentTransform);

        void processMesh(
            ModelMesh& modelMesh,
            const std::map<size_t, ki::material_id>& materialMapping,
            size_t meshIndex,
            const aiNode* node,
            const aiMesh* mesh,
            int nodeLevel);

        void processMeshFace(
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t faceIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiFace* face,
            int nodeLevel);

        void processMeshBone(
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t boneIndex,
            size_t vertexOffset,
            const aiMesh* mesh,
            const aiBone* bone,
            int nodeLevel);

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
