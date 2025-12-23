#include "NodeTree.h"

#include <iostream>

#include <glm/gtx/matrix_decompose.hpp>
#include <fmt/format.h>

#include <assimp/scene.h>

#include "util/glm_format.h"
#include "util/glm_util.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/util_join.h"

#include "util/assimp_util.h"

namespace
{
    const std::string ASSIMP_FBX{"$AssimpFbx$"};

    int calculateNeededSize(const aiNode* node)
    {
        int size = 0;
        for (int childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
            size += calculateNeededSize(node->mChildren[childIndex]);
        }

        return size;
    }

    mesh_set::FbxTransformType resolveFbxTransformType(
        const std::string& transformName)
    {
        if (transformName == "PreRotation") {
            return mesh_set::FbxTransformType::pre_rotation;
        }
        else if (transformName == "Rotation") {
            return mesh_set::FbxTransformType::rotation;
        }
        else if (transformName == "PostRotation") {
            return mesh_set::FbxTransformType::post_rotation;
        }
        else if (transformName == "Scaling") {
            return mesh_set::FbxTransformType::scaling;
        }
        else if (transformName == "Translation") {
            return mesh_set::FbxTransformType::translation;
        }
        else {
            throw fmt::format("unknown fbx_transform: {}", transformName);
        }
    }

    void parseAssimpFbxInfo(mesh_set::TreeNode& treeNode)
    {
        const std::string marker = "_$AssimpFbx$_";
        size_t pos = treeNode.name.find(marker);

        if (pos != std::string::npos) {
            treeNode.isAssimpFbxNode = true;
            treeNode.baseName = treeNode.name.substr(0, pos);
            treeNode.fbxTransformType = resolveFbxTransformType(treeNode.name.substr(pos + marker.length()));
        }
        else {
            treeNode.isAssimpFbxNode = false;
            treeNode.baseName = treeNode.name;
            treeNode.fbxTransformType = mesh_set::FbxTransformType::none;
        }
    }
}

namespace mesh_set
{
    NodeTree::NodeTree(const aiScene* scene)
        : m_scene{scene}
    {}

    const TreeNode* NodeTree::getNode(int index) const
    {
        if (index < 0 || index > m_treeNodes.size()) return nullptr;

        return &m_treeNodes[index];
    }

    const TreeNode* NodeTree::findParent(const TreeNode* treeNode) const
    {
        if (!treeNode) return nullptr;
        if (treeNode->parentIndex < 0) return nullptr;
        return &m_treeNodes[treeNode->parentIndex];
    }

    const std::vector<int>* NodeTree::findChildrenIds(const TreeNode* treeNode) const
    {
        if (!treeNode) return nullptr;

        const auto& it = m_children.find(treeNode->index);
        return it != m_children.end() ? &it->second : nullptr;
    }

    const TreeNode* NodeTree::findByName(const std::string& name) const
    {
        const auto& it = m_nameToNodeIndex.find(name);
        if (it == m_nameToNodeIndex.end()) return nullptr;

        return &m_treeNodes[it->second];
    }

    const TreeNode* NodeTree::findByNode(const aiNode* node) const
    {
        const auto& it = m_nodeToNodeIndex.find(node);
        if (it == m_nodeToNodeIndex.end()) return nullptr;

        return &m_treeNodes[it->second];
    }

    void NodeTree::build()
    {
        {
            auto size = calculateNeededSize(m_scene->mRootNode);
            m_treeNodes.reserve(size);
            m_nameToNodeIndex.reserve(size);
            m_nodeToNodeIndex.reserve(size);
        }

        collectNodes(
            m_scene->mRootNode,
            0,
            -1,
            glm::mat4{1.f});

        for (int meshIndex = 0; meshIndex < m_scene->mNumMeshes; meshIndex++) {
            const auto* mesh = m_scene->mMeshes[meshIndex];
            for (int jointIndex = 0; jointIndex < mesh->mNumBones; jointIndex++) {
                const auto* joint = mesh->mBones[jointIndex];
                const auto& nodeName = assimp_util::normalizeName(joint->mName);

                if (const auto& it = m_nameToNodeIndex.find(nodeName); it != m_nameToNodeIndex.end()) {
                    auto& treeNode = m_treeNodes[it->second];
                    treeNode.jointCount++;
                }
            }
        }
    }

    void NodeTree::collectNodes(
        const aiNode* node,
        int16_t level,
        int16_t parentIndex,
        const glm::mat4& parentTransform)
    {
        int nodeIndex = m_treeNodes.size();
        glm::mat4 globalTransform;
        {
            const auto& nodeName = assimp_util::normalizeName(node->mName);

            auto& treeNode = m_treeNodes.emplace_back();
            treeNode.index = nodeIndex;
            treeNode.parentIndex = parentIndex;
            treeNode.level = level;
            treeNode.name = nodeName;
            treeNode.node = node;
            treeNode.transform = assimp_util::toMat4(node->mTransformation);
            treeNode.globalTransform = parentTransform * treeNode.transform;
            parseAssimpFbxInfo(treeNode);

            {
                treeNode.meshes.reserve(node->mNumMeshes);
                for (int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++) {
                    const auto* mesh = m_scene->mMeshes[node->mMeshes[meshIndex]];
                    treeNode.meshes.push_back(mesh);
                }
            }

            m_nameToNodeIndex.insert({ treeNode.name, nodeIndex });
            m_nodeToNodeIndex.insert( {treeNode.node, nodeIndex });
            m_children[parentIndex].push_back(nodeIndex);

            if (parentIndex >= 0) {
                const auto& parent = m_treeNodes[parentIndex];
                treeNode.hasAssimpFbxNodeParent = parent.isAssimpFbxNode || parent.hasAssimpFbxNodeParent;
            }

            globalTransform = treeNode.globalTransform;
        }

        for (int childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
            collectNodes(
                node->mChildren[childIndex],
                level + 1,
                nodeIndex,
                globalTransform);
        }

        m_treeNodes[nodeIndex].treeSize = m_treeNodes.size() - nodeIndex;
    }

    std::string NodeTree::formatTree()
    {
        std::string sb;
        sb.reserve(10000);

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "    ";
            }
            sb += line;
            sb += "\n";
        };

        for (const auto& treeNode : m_treeNodes) {
            auto meshesSb = util::join(
                treeNode.meshes, ", ",
                [](const auto* mesh) {
                return assimp_util::normalizeName(mesh->mName);
            });

            std::string line = fmt::format(
                "[{}] {}.{}: {}, treeSize={}{}",
                treeNode.level,
                treeNode.parentIndex, treeNode.index, treeNode.name, treeNode.treeSize,
                treeNode.jointCount == 0 ? "" : fmt::format(", joints={}", treeNode.jointCount),
                treeNode.meshes.empty() ? "" : fmt::format(", meshes=[{}]", meshesSb)
            );
            appendLine(sb, treeNode.level, line);
        }

        return sb;
    }

    void NodeTree::dumpTree()
    {
        std::cout << formatTree();
    }
}
