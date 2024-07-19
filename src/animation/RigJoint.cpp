#include "RigJoint.h"

#include <regex>

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace {
    std::regex RE_ASSIMP_FBX{ ".*\\$AssimpFbx\\$.*" };
}

namespace animation {
    RigJoint::RigJoint(const aiNode* node)
        : m_name{ node->mName.C_Str() },
        m_assimpFbx{ std::regex_match(m_name, RE_ASSIMP_FBX) },
        m_index{ -1 },
        m_parentIndex{ -1 },
        m_transform{ assimp_util::toMat4(node->mTransformation) },
        m_invTransform{ glm::inverse(m_transform)},
        m_globalTransform{ 1.f },
        m_globalInvTransform{ 1.f }
    {
    }
}
