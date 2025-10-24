#pragma once

#include "glm/glm.hpp"

#include "animation/VertexJoint.h"

namespace mesh {
#pragma pack(push, 1)
    struct JointEntry {
        JointEntry()
            : m_jointIds{ 0 },
            m_weights{ 0.f }
        {}

        JointEntry(const animation::VertexJoint& v)
        {
            m_jointIds = v.m_jointIds;
            m_weights = v.m_weights;
        }

        glm::uvec4 m_jointIds;
        glm::vec4 m_weights;
    };
#pragma pack(pop)
}
