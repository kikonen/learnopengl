#pragma once

#include <functional>

#include "kigl/kigl.h"

#include <stdint.h>

namespace model
{
    class Node;
    class NodeType;
}


namespace render
{
    struct DrawContext
    {
        const std::function<bool(const model::Node*)>& nodeSelector;

        // rnder::KIND_NONE
        const uint8_t kindBits;

        // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        const GLbitfield copyMask;
    };
}
