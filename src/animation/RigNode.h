#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct RigNode {
        RigNode(const aiNode* node);

        const std::string m_name;
        const bool m_assimpFbx;

        int16_t m_index;
        int16_t m_parentIndex;

        // NOTE KI for debug
        int16_t m_level{ -1 };

        int16_t m_jointIndex{ -1 };
        bool m_jointRequired : 1{ false };

        int16_t m_socketIndex{ -1 };
        bool m_socketRequired : 1{ false };

        bool m_mesh{ false };

        // local == relative to parent joint
        const glm::mat4 m_transform;
        const glm::mat4 m_invTransform;

        // global == relative to model
        glm::mat4 m_globalTransform;
        glm::mat4 m_globalInvTransform;
    };
}
