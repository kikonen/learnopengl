#pragma once

#include "kigl/kigl.h"

namespace render
{
    struct MeshInfoSSBO
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        int32_t  baseVertex;
        uint32_t maxInstances;
        uint32_t outputOffset;
    };
}
