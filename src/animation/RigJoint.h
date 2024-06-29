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

        // NOTE KI for debug
        int16_t m_level{ -1 };

        int16_t m_boneIndex{ -1 };
        bool m_boneRequired : 1{ false };

        int16_t m_socketIndex{ -1 };
        bool m_socketRequired : 1{ false };

        // local == relative to parent joint
        const glm::mat4 m_transform;
        const glm::mat4 m_invTransform;

        // global == relative to model
        glm::mat4 m_globalTransform;
        glm::mat4 m_globalInvTransform;
    };
}
