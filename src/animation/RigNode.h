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

        // TODO KI this is WRONG
        // => multiple Joints can be bound to same RigNode
        int16_t m_jointIndex{ -1 };
        bool m_jointRequired : 1{ false };

        // TODO KI this is WRONG
        // => multiple Sockets can be bound to same RigNode
        int16_t m_socketIndex{ -1 };
        bool m_socketRequired : 1{ false };

        bool m_mesh{ false };

        // local == relative to parent joint
        const glm::mat4 m_transform;

        // global == relative to model
        // => used for non-animated models
        glm::mat4 m_globalTransform;
    };
}
