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
        void bindJoint(int16_t boneIndex, int16_t jointIndex) noexcept;

        const animation::BoneInfo* getInfo(int16_t boneIndex) const noexcept;

        // @return boneInfo, null if not found
        const animation::BoneInfo* findByJointIndex(int16_t jointIndex) const noexcept;

        inline bool hasBones() const noexcept {
            return !m_jointNameToIndex.empty();
        }

        inline int16_t size() const noexcept {
            return static_cast<int16_t>(m_jointNameToIndex.size());
        }

        std::vector<animation::BoneInfo> m_boneInfos;
        std::map<std::string, int16_t> m_jointNameToIndex;

        std::map<int16_t, int16_t> m_jointToBone;
    };
}
