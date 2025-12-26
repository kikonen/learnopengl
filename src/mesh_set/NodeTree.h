#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include <glm/glm.hpp>

struct aiScene;
struct aiMesh;
struct aiNode;
struct aiBone;

namespace mesh_set
{
    enum class FbxTransformType
    {
        none,
        pre_rotation,
        rotation,
        geometric_rotation,
        geometric_rotation_inverse,
        post_rotation,
        scaling,
        translation,
    };

    struct TreeNode
    {
        int index;
        int parentIndex;
        int level;
        std::string name;
        std::string baseName;
        FbxTransformType fbxTransformType;
        bool isAssimpFbxNode;
        bool hasAssimpFbxNodeParent;
        const aiNode* node;
        std::vector<const aiMesh*> meshes;
        glm::mat4 transform;
        glm::mat4 globalTransform;
        int treeSize;

        std::set<const aiBone*> joints;

        // Link to the actual bone node (for $AssimpFbx$ nodes)
        int linkedNodeIndex = -1;
    };

    struct NodeTree
    {
        std::vector<TreeNode> m_treeNodes;
        std::unordered_map<std::string, int> m_nameToNodeIndex;
        std::unordered_map<const aiNode*, int> m_nodeToNodeIndex;

        std::unordered_map<int, std::vector<int>> m_children;

        const aiScene* m_scene;

        NodeTree(const aiScene* scene);

        const TreeNode* getNode(int index) const;

        const TreeNode* findParent(const TreeNode* treeNode) const;

        const std::vector<int>* findChildrenIds(const TreeNode* treeNode) const;

        const TreeNode* findByName(const std::string& name) const;

        const TreeNode* findByNode(const aiNode* node) const;

        void build();

        void collectNodes(
            const aiNode* node,
            int16_t level,
            int16_t parentIndex,
            const glm::mat4& parentTransform);

        std::string formatTree();

        void dumpTree();
    };
}
