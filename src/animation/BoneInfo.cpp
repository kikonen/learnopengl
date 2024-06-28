#include "BoneInfo.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    BoneInfo::BoneInfo(const aiBone* bone)
        : m_offsetMatrix{ assimp_util::toMat4(bone->mOffsetMatrix) },
        m_jointName{ bone->mName.C_Str() }
    {}

}
