#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

namespace render
{
    // Per-instance input data (read-only during culling)
    struct InstanceDataSSBO
    {
        glm::vec4 u_boundingSphere;    // xyz = center (world space), w = radius
        glm::vec4 u_transform0;        // mat4 column 0 (or use quaternion + position)
        glm::vec4 u_transform1;        // mat4 column 1
        glm::vec4 u_transform2;        // mat4 column 2
        glm::vec4 u_transform3;        // mat4 column 3 (position in xyz, scale in w if uniform)
        GLuint u_meshId;            // which mesh this instance uses
        GLuint u_flags;             // NO_CULL, ALWAYS_VISIBLE, etc.
        GLuint _pad0;
        GLuint _pad1;
    };  // 96 bytes, aligned
}
