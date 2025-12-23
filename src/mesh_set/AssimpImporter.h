#pragma once

#include <map>
#include <memory>
#include <atomic>

#include "MeshSetImporter.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiFace;
struct aiBone;
struct aiMetadata;

namespace animation
{
	struct Rig;
    struct RigNode;
	struct JointContainer;
}

namespace mesh
{
    class ModelMesh;
    class MeshSet;
}

namespace mesh_set
{
    struct LoadContext;

    struct SkeletonSet;

    class AssimpImporter : public MeshSetImporter
    {
    public:
        AssimpImporter(
            const std::shared_ptr<std::atomic_bool>& alive,
            bool debug);

        ~AssimpImporter();

    protected:
        void loadData(
            mesh::MeshSet& meshSet);

        void loadResolvedPath(
            mesh::MeshSet& meshSet);

    private:
        void loadAnimations(
            LoadContext& ctx,
            animation::Rig& rig,
            const std::string& namePrefix,
            const std::string& filePath,
            const aiScene* scene);

        void processMeshes(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
            const SkeletonSet& skeletonSet,
            const aiScene* scene,
            const aiNode* node);

        void processMesh(
            LoadContext& ctx,
            mesh::MeshSet& meshSet,
			animation::Rig* rig,
			mesh::ModelMesh& modelMesh,
			const aiNode* node,
			const aiMesh* mesh);

        void processMeshFace(
            LoadContext& ctx,
            mesh::ModelMesh& modelMesh,
            size_t faceIndex,
            const aiMesh* mesh,
            const aiFace* face);

        void processMeshBone(
			animation::Rig& rig,
			animation::JointContainer& jointContainer,
			mesh::ModelMesh& modelMesh,
            const aiBone* bone);

    private:
        const bool m_debug;
    };
}
