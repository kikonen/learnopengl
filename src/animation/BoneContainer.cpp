#include "BoneContainer.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

#include "BoneInfo.h"

namespace animation {
    bool BoneContainer::empty() const noexcept
    {
        return m_boneInfos.empty();
    }

    BoneInfo& BoneContainer::registerBone(const aiBone* bone) noexcept
    {
        int16_t index;

        std::string jointName{ bone->mName.C_Str() };
        const auto& it = m_jointNameToIndex.find(jointName);
        if (it != m_jointNameToIndex.end()) {
            index = it->second;
        }
        else {
            index = static_cast<int16_t>(m_jointNameToIndex.size());
            auto& bi = m_boneInfos.emplace_back(bone);
            bi.m_index = index;

            m_jointNameToIndex.insert({ jointName, index });
        }

        return m_boneInfos[index];
    }

    void BoneContainer::bindJoint(int16_t boneIndex, int16_t jointIndex) noexcept
    {
        auto& bi = m_boneInfos[boneIndex];
        bi.m_jointIndex = jointIndex;
        m_jointToBone.insert({ jointIndex, boneIndex});
    }

    const animation::BoneInfo* BoneContainer::getInfo(int16_t boneIndex) const noexcept
    {
        return boneIndex >= 0 ? &m_boneInfos[boneIndex] : nullptr;
    }

    const animation::BoneInfo* BoneContainer::findByJointIndex(int16_t jointIndex) const noexcept
    {
        const auto& it = m_jointToBone.find(jointIndex);
        return it != m_jointToBone.end() ? &m_boneInfos[it->second] : nullptr;
    }

}
