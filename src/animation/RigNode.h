#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct RigNode {
        RigNode(const aiNode* node);

        const aiNode* m_node;

        const std::string m_name;

        int16_t m_index;
        int16_t m_parentIndex;

        glm::mat4 m_transform;
        glm::mat4 m_localTransform;
    };
}
