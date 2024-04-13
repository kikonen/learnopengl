#pragma once

#include "glm/glm.hpp"

#include "BoneBinding.h"

namespace mesh {
    struct BoneEntry {
        BoneEntry()
            : m_boneIds{ 0 },
            m_weights{ 0.f }
        {}

        BoneEntry(const BoneBinding& v)
        {
            memcpy(m_boneIds, v.m_boneIds, sizeof(m_boneIds));
            memcpy(m_weights, v.m_weights, sizeof(m_weights));
        }

        uint32_t m_boneIds[MAX_VERTEX_BONE_COUNT];
        float m_weights[MAX_VERTEX_BONE_COUNT];
    };
}
