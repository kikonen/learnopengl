#include "Joint.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    Joint::Joint(
        const aiBone* bone,
        int16_t jointIndex)
        : m_nodeName{assimp_util::normalizeName(bone->mName) },
        m_offsetMatrix{assimp_util::toMat4(bone->mOffsetMatrix)},
        m_jointIndex{ jointIndex }
    {}
}
