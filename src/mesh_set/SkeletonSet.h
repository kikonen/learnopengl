#pragma once

#include <string>
#include <vector>
#include <set>
#include <queue>
#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiBone;

namespace animation
{
    struct Rig;
    struct JointContainer;
}

namespace mesh_set
{
    struct NodeTree;

    struct RiggedSkeleton
    {
        int index;
        std::set<const aiNode*> jointNodes;
        const aiNode* skeletonRoot;
        const aiNode* rigRoot;

        std::vector<const aiMesh*> meshes;
    };

    struct SkeletonExtract
    {
        std::vector<RiggedSkeleton> m_riggedSkeletons;
        std::unordered_map<const aiMesh*, int> m_meshRiggedSkeleton;
    };

    struct Skeleton
    {
        int index;
        std::string name;
        const aiNode* skeletonRoot;
        const aiNode* rigRoot;

        std::shared_ptr<animation::Rig> toRig(
            const aiScene* scene,
            const mesh_set::NodeTree& tree) const;
    };

    struct MeshAssociation
    {
        const aiMesh* mesh;
        int skeletonIndex;
    };

    class SkeletonSet
    {
        friend class AssimpImporter;

    public:
        void resolve(const aiScene* scene);
        std::shared_ptr<animation::Rig> findRig(const aiMesh* mesh) const;

        const mesh_set::NodeTree& getTree() const;

    private:
        void buildSkeletons();
        void buildRigs(const aiScene* scene);

    private:
        std::shared_ptr<NodeTree> m_tree;

        std::shared_ptr<mesh_set::SkeletonExtract> m_extract;

        std::vector<mesh_set::Skeleton> m_skeletons;
        std::vector<MeshAssociation> m_meshAssociations;

        std::vector<std::shared_ptr<animation::Rig>> m_rigs;
    };
}
