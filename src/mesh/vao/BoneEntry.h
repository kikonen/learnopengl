#pragma once

#include "glm/glm.hpp"

#include "animation/VertexBone.h"

namespace mesh {
#pragma pack(push, 1)
    struct BoneEntry {
        BoneEntry()
            : m_boneIds{ 0 },
            m_weights{ 0.f }
        {}

        BoneEntry(const animation::VertexBone& v)
        {
            m_boneIds = v.m_boneIds;
            m_weights = v.m_weights;
        }

        glm::uvec4 m_boneIds;
        glm::vec4 m_weights;
    };
#pragma pack(pop)
}
