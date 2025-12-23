#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiBone;

namespace animation {
    // NOTE KI Assimp aiBone == Joint
    struct Joint {
        Joint(
            const aiBone* bone,
            int16_t jointIndex,
            int16_t nodeIndex);

        std::string m_nodeName;
        int16_t m_jointIndex;
        int16_t m_nodeIndex;
        glm::mat4 m_offsetMatrix;
    };
}
