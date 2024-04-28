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

        int16_t m_id;
        int16_t m_parentId;

        glm::mat4 m_transform;
    };
}
