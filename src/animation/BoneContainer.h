#pragma once

#include <string>
#include <vector>
#include <map>

#include "ki/size.h"

#include "BoneTransform.h"

struct aiBone;

namespace animation {
    // Manage bones shared based into their name
    struct BoneContainer {
        bool valid() const noexcept {
            return !m_bones.empty();
        }

        std::vector<animation::BoneTransform> m_bones;

        uint16_t getBoneId(const aiBone* bone) noexcept;

        std::map<std::string, uint16_t> m_boneNameToIndex;
    };
}
