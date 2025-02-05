#pragma once

#include <functional>

#include "kigl/kigl.h"

#include <stdint.h>

class Node;

namespace mesh {
    class MeshType;
}

namespace render
{
    struct DrawContext
    {
        const std::function<bool(const mesh::MeshType*)>& typeSelector;
        const std::function<bool(const Node*)>& nodeSelector;

        const uint8_t kindBits;
        const GLbitfield copyMask;
    };
}
