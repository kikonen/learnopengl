#pragma once

#include <string>
#include <vector>
#include <map>

#include "ki/size.h"

struct aiBone;

namespace animation {
    struct BoneInfo;

    // Manage bones shared based into their name
    struct BoneContainer {
        bool empty() const noexcept;

        animation::BoneInfo& registerBone(const aiBone* bone) noexcept;
        void bindNode(int16_t boneIndex, int16_t nodeIndex) noexcept;

        const animation::BoneInfo* getNode(int16_t boneIndex) const noexcept;

        // @return boneInfo, null if not found
        const animation::BoneInfo* findByNodeIndex(int16_t nodeIndex) const noexcept;

        inline bool hasBones() const noexcept {
            return !m_nodeNameToIndex.empty();
        }

        inline int16_t size() const noexcept {
            return static_cast<int16_t>(m_nodeNameToIndex.size());
        }

        std::vector<animation::BoneInfo> m_boneInfos;
        std::map<std::string, int16_t> m_nodeNameToIndex;

        std::map<int16_t, int16_t> m_nodeToBone;
    };
}
