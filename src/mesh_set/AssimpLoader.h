#pragma once

#include <map>
#include <memory>
#include <atomic>

#include "MeshSetLoader.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiFace;
struct aiBone;
struct aiMetadata;

namespace animation
{
    struct RigNode;
}

namespace mesh
{
    class ModelMesh;
    class MeshSet;
}

namespace mesh_set
{
    struct LoadContext;

    class AssimpLoader : public MeshSetLoader
    {
    public:
        AssimpLoader(
            const std::shared_ptr<std::atomic_bool>& alive,
            bool debug);

        ~AssimpLoader();

    protected:
        void loadData(
            mesh::MeshSet& meshSet);

        void loadResolvedPath(
            mesh::MeshSet& meshSet);

    private:
        void collectNodes(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
            std::vector<const aiNode*>& assimpNodes,
            const aiScene* scene,
            const aiNode* node,
            int16_t level,
            int16_t parentIndex,
            const glm::mat4& parentTransform);

        void dumpMetaData(
            mesh::MeshSet& meshSet,
            const std::vector<animation::RigNode>& nodes,
            const std::vector<const aiNode*>& assimpNodes);

        void dumpMetaData(
            mesh::MeshSet& meshSet,
            const animation::RigNode& RigNode,
            const aiNode* node);

        void loadAnimations(
            LoadContext& ctx,
            const std::string& namePrefix,
            const std::string& filePath,
            const aiScene* scene);

        void processMeshes(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
            const std::vector<const aiNode*>& assimpNodes,
            const aiScene* scene);

        void processMesh(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
            animation::RigNode& RigNode,
            mesh::ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh);

        void processMeshFace(
            LoadContext& ctx,
            mesh::ModelMesh& modelMesh,
            size_t meshIndex,
            size_t faceIndex,
            const aiMesh* mesh,
            const aiFace* face);

        void processMeshBone(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
            mesh::ModelMesh& modelMesh,
            size_t meshIndex,
            const aiMesh* mesh,
            const aiBone* bone);

    private:
        const bool m_debug;
    };
}
