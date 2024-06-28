#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct RigJoint {
        RigJoint(const aiNode* node);

        const std::string m_name;

        int16_t m_index;
        int16_t m_parentIndex;

        int16_t m_boneIndex{ -1 };
        bool m_boneRequired : 1{ false };
        bool m_socketRequired : 1{ false };

        // local == relative to parent joint
        const glm::mat4 m_localTransform;
        // global == relative to model
        glm::mat4 m_globalTransform;
    };
}