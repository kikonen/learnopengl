#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

namespace render
{
    // Compacted visible instance (output)
    struct VisibleInstanceSSBO
    {
        glm::vec4 u_transform0;
        glm::vec4 u_transform1;
        glm::vec4 u_transform2;
        glm::vec4 u_transform3;
    };  // 64 bytes - only what vertex shader needs
}
