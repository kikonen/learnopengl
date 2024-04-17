#include "BoneContainer.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "util/assimp_util.h"

namespace animation {
    uint16_t BoneContainer::getBoneId(const aiBone* bone) noexcept
    {
        uint16_t index;

        const auto& it = m_boneNameToIndex.find(bone->mName.C_Str());
        if (it != m_boneNameToIndex.end()) {
            index = it->second;
        }
        else {
            index = static_cast<uint16_t>(m_bones.size());
            m_boneNameToIndex.insert({ bone->mName.C_Str(), index });
            m_bones.emplace_back(assimp_util::toMat4(bone->mOffsetMatrix));
        }
        return index;
    }
}
