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

        // NOTE KI for debug
        int16_t m_level{ -1 };

        bool m_requiredForJoint : 1{ false };
        bool m_requiredForSocket : 1{ false };

        bool m_hasJoint : 1{ false };
        bool m_hasSocket : 1{ false };

        // sockets are shared in rig
        int16_t m_socketIndex{ -1 };

        // NOTE KI for debug
        // true if mesh is bound to this node
        bool m_hasMesh{ false };

        // local == relative to parent joint
        const glm::mat4 m_transform;

        // global == relative to model
        // => used for non-animated models
        glm::mat4 m_globalTransform;

        ///**
        // * FROM ASSIMP (aiBone)
        // * Matrix that transforms from mesh space to bone space in bind pose.
        // *
        // * This matrix describes the position of the mesh
        // * in the local space of this bone when the skeleton was bound.
        // * Thus it can be used directly to determine a desired vertex position,
        // * given the world-space transform of the bone when animated,
        // * and the position of the vertex in mesh space.
        // *
        // * It is sometimes called an inverse-bind matrix,
        // * or inverse bind pose matrix.
        // */
        //glm::mat4 m_offsetMatrix{ 1.f };

        const bool m_assimpFbx;
    };
}
