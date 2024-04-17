#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct AnimationNode {
        AnimationNode(aiNode* node);

        int16_t m_parentIndex;

        glm::mat4 m_transform;
    };
}
