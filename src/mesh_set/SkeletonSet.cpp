#include "SkeletonSet.h"

#include <map>
#include <set>
#include <iostream>

#include <glm/gtx/matrix_decompose.hpp>
#include <fmt/format.h>

#include <assimp/scene.h>

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/JointContainer.h"

#include "util/glm_format.h"
#include "util/glm_util.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/util_join.h"

#include "util/assimp_util.h"

#include "NodeTree.h"

namespace
{
    struct SkeletonExtraction
    {
        std::unordered_map<const aiNode*, std::vector<const aiMesh*>> skeletonRoots;
        std::unordered_map<const aiNode*, std::vector<const aiMesh*>> rigRoots;

        std::unordered_map<const aiMesh*, const aiNode*> meshSkeletonRoots;
        std::unordered_map<const aiMesh*, const aiNode*> meshRigRoots;
    };
}

namespace
{
    const aiNode* findAnimationRoot(
        const mesh_set::NodeTree& tree,
        const aiNode* skeletonRoot)
    {
        const auto* curr = tree.findByNode(skeletonRoot);
        if (!curr) return skeletonRoot;

        while (curr->hasAssimpFbxNodeParent) {
            curr = tree.findParent(curr);
        }
        return curr->node;
    }

    const aiNode* findSkeletonRoot(
        const aiScene* scene,
        const mesh_set::NodeTree& tree,
        const std::set<const aiNode*>& jointNodes)
    {
        if (jointNodes.empty()) return nullptr;

        std::set<const mesh_set::TreeNode*> current;

        for (const auto& node : jointNodes) {
            const auto* treeNode = tree.findByNode(node);

            if (!treeNode) {
                KI_INFO_OUT(fmt::format(
                    "joint_node_missing={}",
                    assimp_util::normalizeName(node->mName)));
                continue;
            }

            current.insert(treeNode);
        }

        while (current.size() > 1) {
            int maxLevel = 0;
            for (const auto* treeNode : current) {
                maxLevel = std::max(maxLevel, treeNode->level);
            }

            std::set<const mesh_set::TreeNode*> next;

            for (const auto* treeNode : current) {
                if (treeNode->level == maxLevel) {
                    const auto* parent = tree.findParent(treeNode);
                    if (parent) {
                        next.insert(parent);
                    }
                    else {
                        next.insert(treeNode);
                    }
                }
                else {
                    next.insert(treeNode);
                }
            }

            if (next == current) break;
            current = next;
        }

        const mesh_set::TreeNode* rootTreeNode{ nullptr };

        if (current.size() == 1) {
            rootTreeNode = *current.begin();
        }

        KI_INFO_OUT(fmt::format(
            "found_root={}",
            rootTreeNode ? rootTreeNode->name : "<NULL>"));

        return rootTreeNode ? rootTreeNode->node : nullptr;
    }

    std::set<const aiNode*> collectJointNodes(
        const aiScene* scene,
        const mesh_set::NodeTree& tree,
        const aiMesh* mesh)
    {
        std::set<const aiNode*> jointNodes;

        for (unsigned int j = 0; j < mesh->mNumBones; j++) {
            const auto& nodeName = assimp_util::normalizeName(mesh->mBones[j]->mName);
            const auto* treeNode = tree.findByName(nodeName);
            if (treeNode) {
                jointNodes.insert(treeNode->node);
            }
            else {
                KI_INFO_OUT(fmt::format("ERROR: JOINT_NOT_FOUND: {}", nodeName));
            }
        }

        return jointNodes;
    }
}

namespace mesh_set
{
    std::shared_ptr<SkeletonExtract> extractSkeletons(
        const aiScene* scene,
        const mesh_set::NodeTree& tree)
    {
        std::shared_ptr<mesh_set::SkeletonExtract> result = std::make_shared<mesh_set::SkeletonExtract>();

        std::map<std::set<const aiNode*>, int> foundSkeletons;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[i];

            const auto jointNodes = collectJointNodes(scene, tree, mesh);
            if (jointNodes.empty()) continue;

            int skeletonIndex;

            const auto& it = foundSkeletons.find(jointNodes);
            if (it == foundSkeletons.end()) {
                skeletonIndex = static_cast<int>(result->m_riggedSkeletons.size());
                auto& riggedSkeleton = result->m_riggedSkeletons.emplace_back();

                riggedSkeleton.index = skeletonIndex;
                riggedSkeleton.jointNodes = jointNodes;

                riggedSkeleton.skeletonRoot = findSkeletonRoot(
                    scene,
                    tree,
                    riggedSkeleton.jointNodes);

                riggedSkeleton.rigRoot = findAnimationRoot(
                    tree,
                    riggedSkeleton.skeletonRoot);

                foundSkeletons.insert({ riggedSkeleton.jointNodes, skeletonIndex });

                riggedSkeleton.meshes.push_back(mesh);
            }
            else {
                skeletonIndex = it->second;

                auto& riggedSkeleton = result->m_riggedSkeletons[skeletonIndex];
                riggedSkeleton.meshes.push_back(mesh);
            }

            result->m_meshRiggedSkeleton.insert({ mesh, skeletonIndex });
        }

        {
            for (const auto& riggedSkeleton : result->m_riggedSkeletons) {
                KI_INFO_OUT(fmt::format(
                    "RIGGED: index={}, root={}, rigRoot={}",
                    riggedSkeleton.index,
                    assimp_util::normalizeName(riggedSkeleton.skeletonRoot->mName),
                    assimp_util::normalizeName(riggedSkeleton.rigRoot->mName)));
            }

            for (const auto& [mesh, skeletonIndex] : result->m_meshRiggedSkeleton) {
                const auto& riggedSkeleton = result->m_riggedSkeletons[skeletonIndex];

                KI_INFO_OUT(fmt::format(
                    "MESH: {}: skeleton={}, [{}]",
                    assimp_util::normalizeName(mesh->mName),
                    skeletonIndex,
                    assimp_util::normalizeName(riggedSkeleton.rigRoot->mName)));
            }
        }

        return result;
    }
}

namespace mesh_set
{
    struct QueueEntry
    {
        int parentIndex;
        int level;
        const mesh_set::TreeNode* treeNode;
        glm::mat4 parentTransform;
    };

    std::shared_ptr<animation::Rig> Skeleton::toRig(
        const aiScene* scene,
        const mesh_set::NodeTree& tree) const
    {
        auto rig = std::make_shared<animation::Rig>();
        rig->m_name = name;
        rig->m_skeletonNode = assimp_util::normalizeName(skeletonRoot->mName);

        const auto* rootTreeNode = tree.findByNode(rigRoot);
        if (!rootTreeNode) return nullptr;

        std::queue<QueueEntry> queue;
        queue.push({ -1, 0, rootTreeNode, glm::mat4{ 1.f } });

        while (!queue.empty()) {
            const auto entry = queue.front();
            queue.pop();

            const auto* treeNode = entry.treeNode;
            const auto* node = treeNode->node;

            glm::mat4 globalTransform;
            int nodeIndex = static_cast<int>(rig->m_nodes.size());
            {
                auto& rigNode = rig->m_nodes.emplace_back(node);

                rigNode.m_index = nodeIndex;
                rigNode.m_level = entry.level;
                rigNode.m_parentIndex = entry.parentIndex;

                rigNode.m_globalTransform = entry.parentTransform * rigNode.m_transform;
                globalTransform = rigNode.m_globalTransform;
            }

            const auto* childrenIds = tree.findChildrenIds(treeNode);
            if (childrenIds) {
                for (const auto& childIndex : *childrenIds) {
                    queue.push({ nodeIndex, entry.level + 1, tree.getNode(childIndex), globalTransform });
                }
            }
        }

        return rig;
    }

    void SkeletonSet::resolve(const aiScene* scene)
    {
        m_tree = std::make_shared<mesh_set::NodeTree>(scene);
        {
            auto& tree = *m_tree;
            tree.build();
            tree.dumpTree();
        }

        m_extract = extractSkeletons(scene, *m_tree);

        buildSkeletons();
        buildRigs(scene);
    }

    void SkeletonSet::buildSkeletons()
    {
        m_skeletons.reserve(m_extract->m_riggedSkeletons.size());

        for (const auto& riggedSkeleton : m_extract->m_riggedSkeletons) {
            auto skeletonIndex = static_cast<int>(m_skeletons.size());
            {
                auto& skeleton = m_skeletons.emplace_back();
                skeleton.index = skeletonIndex;
                skeleton.rigRoot = riggedSkeleton.rigRoot;
                skeleton.skeletonRoot = riggedSkeleton.skeletonRoot;
                skeleton.name = assimp_util::normalizeName(skeleton.rigRoot->mName);
            }

            for (const auto* mesh : riggedSkeleton.meshes) {
                auto& meshSkeleton = m_meshAssociations.emplace_back();
                meshSkeleton.mesh = mesh;
                meshSkeleton.skeletonIndex = skeletonIndex;
            }
        }
    }

    void SkeletonSet::buildRigs(const aiScene* scene)
    {
        for (const auto& skeleton : m_skeletons) {
            m_rigs.push_back(skeleton.toRig(scene, *m_tree));
        }
    }

    std::shared_ptr<animation::Rig> SkeletonSet::findRig(const aiMesh* mesh) const
    {
        for (const auto& assoc : m_meshAssociations)
        {
            if (assoc.mesh == mesh)
                return m_rigs[assoc.skeletonIndex];
        }
        return nullptr;
    }

    const mesh_set::NodeTree& SkeletonSet::getTree() const
    {
        return *m_tree;
    }
}
