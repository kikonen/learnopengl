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

        std::string nodeName{ bone->mName.C_Str() };
        const auto& it = m_nodeNameToIndex.find(nodeName);
        if (it != m_nodeNameToIndex.end()) {
            index = it->second;
        }
        else {
            index = static_cast<int16_t>(m_nodeNameToIndex.size());
            auto& bi = m_boneInfos.emplace_back(bone);
            bi.m_index = index;

            m_nodeNameToIndex.insert({ nodeName, index });
        }

        return m_boneInfos[index];
    }

    void BoneContainer::bindNode(int16_t boneIndex, int16_t nodeIndex) noexcept
    {
        auto& bi = m_boneInfos[boneIndex];
        bi.m_nodeIndex = nodeIndex;
        m_nodeToBone.insert({ nodeIndex, boneIndex});
    }

    const animation::BoneInfo* BoneContainer::getNode(int16_t boneIndex) const noexcept
    {
        return boneIndex >= 0 ? &m_boneInfos[boneIndex] : nullptr;
    }

    const animation::BoneInfo* BoneContainer::findByNodeIndex(int16_t nodeIndex) const noexcept
    {
        const auto& it = m_nodeToBone.find(nodeIndex);
        return it != m_nodeToBone.end() ? &m_boneInfos[it->second] : nullptr;
    }

}
