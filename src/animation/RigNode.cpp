#include "RigNode.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    RigNode::RigNode(const aiNode* node)
        : m_name{ node->mName.C_Str() },
        m_index{ -1 },
        m_parentIndex{ -1 },
        m_localTransform{ assimp_util::toMat4(node->mTransformation) },
        m_globalTransform{ 1.f }
    {
    }
}
