#include "Joint.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    Joint::Joint(const aiBone* bone)
        : m_offsetMatrix{ assimp_util::toMat4(bone->mOffsetMatrix) },
        m_nodeName{ assimp_util::normalizeName(bone->mName.C_Str()) }
    {}

}
