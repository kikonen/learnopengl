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
        inline bool hasBones() const noexcept {
            return !m_offsetMatrices.empty();
        }

        inline uint16_t size() const noexcept {
            return static_cast<uint16_t>(m_boneNameToIndex.size());
        }


        uint16_t getBoneId(const aiBone* bone) noexcept;

        std::vector<glm::mat4> m_offsetMatrices;
        std::map<std::string, uint16_t> m_boneNameToIndex;
    };
}
