#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiBone;

namespace animation {
    struct BoneInfo {
        BoneInfo(const aiBone* bone);

        int16_t m_index{ -1 };
        int16_t m_nodeIndex{ -1 };

        std::string m_nodeName;

        // It is sometimes called an inverse - bind matrix,
        // or inverse bind pose matrix.
        glm::mat4 m_offsetMatrix;
    };
}
