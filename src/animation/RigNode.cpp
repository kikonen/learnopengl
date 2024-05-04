#include "RigNode.h"

#include <assimp/scene.h>

namespace animation {
    RigNode::RigNode(const aiNode* node)
        : m_node{ node },
        m_name{ node->mName.C_Str() },
        m_index{ -1 },
        m_parentIndex{ -1 },
        m_transform{ 1.f },
        m_localTransform{ 1.f }
    {
    }
}
