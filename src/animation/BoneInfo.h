#pragma once

#include <string>
#include <vector>
#include <map>

#include "ki/size.h"

#include "BoneTransform.h"

struct aiBone;

namespace animation {
    struct BoneInfo {
        BoneInfo(const aiBone* bone);

        int16_t m_index{ -1 };

        std::string m_nodeName;
        int16_t m_nodeIndex{ -1 };

        // It is sometimes called an inverse - bind matrix,
        // or inverse bind pose matrix.
        glm::mat4 m_offsetMatrix;
    };
}
