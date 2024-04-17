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
        std::vector<animation::BoneTransform> m_bones;
        std::map<std::string, uint16_t> m_boneNameToIndex;

        uint16_t getBoneId(const aiBone* bone) noexcept;
    };
}
