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
struct aiMetadata;

namespace animation {
    struct RigJoint;
}

namespace mesh {
    struct LoadContext;
    class ModelMesh;

    class AssimpLoader : public ModelLoader {
    public:
        AssimpLoader(
            std::shared_ptr<std::atomic<bool>> alive);

        ~AssimpLoader();

    protected:
        void loadData(
            mesh::MeshSet& meshSet);

        void loadResolvedPath(
            mesh::MeshSet& meshSet);

    private:
        void collectJoints(
            mesh::LoadContext& ctx,
            std::vector<const aiNode*>& assimpNodes,
            const aiScene* scene,
            const aiNode* node,
            int16_t level,
            int16_t parentIndex,
            const glm::mat4& parentTransform,
            const glm::mat4& parentInvTransform);

        void dumpMetaData(
            const std::vector<animation::RigJoint>& joints,
            const std::vector<const aiNode*>& assimpNodes);

        void dumpMetaData(
            const animation::RigJoint& rigJoint,
            const aiNode* node);

        void loadAnimations(
            mesh::LoadContext& ctx,
            const std::string& namePrefix,
            const aiScene* scene);

        void processMeshes(
            mesh::LoadContext& ctx,
            mesh::MeshSet& meshSet,
            const std::vector<const aiNode*>& assimpNodes,
            const aiScene* scene);

        void processMesh(
            mesh::LoadContext& ctx,
            animation::RigJoint& rigJoint,
            ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh);

        void processMeshFace(
            mesh::LoadContext& ctx,
            ModelMesh& modelMesh,
            size_t meshIndex,
            size_t faceIndex,
            const aiMesh* mesh,
            const aiFace* face);

        void processMeshBone(
            mesh::LoadContext& ctx,
            ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh,
            const aiBone* bone);

        void processMaterials(
            const MeshSet& meshSet,
            std::vector<Material>& materials,
            std::map<size_t, ki::material_id>& materialMapping,
            const aiScene* scene);

        Material processMaterial(
            const MeshSet& meshSet,
            const aiScene* scene,
            const aiMaterial* material);

        std::string findTexturePath(
            const MeshSet& meshSet,
            const std::string& origPath);
    };
}
