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
            int16_t jointIndex);

        std::string m_nodeName;
        int16_t m_jointIndex{ 0 };
        int16_t m_nodeIndex{ 0 };
        glm::mat4 m_offsetMatrix{ 1.f };
    };
}
