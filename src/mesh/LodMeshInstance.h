#pragma once

#include <stdint.h>

namespace mesh
{
    struct LodMeshInstance
    {
        uint8_t m_lodMeshIndex;
        uint32_t m_socketBaseIndex;
        uint32_t m_jointBaseIndex;
    };
}
