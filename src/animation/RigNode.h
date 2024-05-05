#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct RigNode {
        RigNode(const aiNode* node);

        const std::string m_name;

        int16_t m_index;
        int16_t m_parentIndex;

        glm::mat4 m_localTransform;
        glm::mat4 m_globalTransform;
    };
}
