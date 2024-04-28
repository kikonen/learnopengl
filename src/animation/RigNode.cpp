#include "RigNode.h"

#include <assimp/scene.h>

namespace animation {
    RigNode::RigNode(const aiNode* node)
        : m_node{ node },
        m_name{ node->mName.C_Str() },
        m_id{ -1 },
        m_parentId{ -1 },
        m_transform{ 1.f }
    {
    }
}
