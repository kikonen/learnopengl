#include "RigJoint.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    RigJoint::RigJoint(const aiNode* node)
        : m_name{ node->mName.C_Str() },
        m_index{ -1 },
        m_parentIndex{ -1 },
        m_transform{ assimp_util::toMat4(node->mTransformation) },
        m_invTransform{ glm::inverse(m_transform)},
        m_globalTransform{ 1.f },
        m_globalInvTransform{ 1.f }
    {
    }
}
