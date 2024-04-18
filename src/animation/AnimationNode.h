#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct AnimationNode {
        AnimationNode(const aiNode* node);

        const aiNode* m_node;

        int16_t m_id;
        int16_t m_parentId;

        glm::mat4 m_transform;
    };
}
