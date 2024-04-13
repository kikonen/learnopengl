#pragma once

#include "ki/size.h"

#define MAX_VERTEX_BONE_COUNT 4

namespace mesh {
    struct BoneBinding {
        uint32_t m_boneIds[MAX_VERTEX_BONE_COUNT] = { 0 };
        float m_weights[MAX_VERTEX_BONE_COUNT] = { 0.f };
    };
}
