#pragma once

#include <stdint.h>

namespace mesh
{
    struct LodMeshInstance
    {
        uint32_t m_socketBaseIndex{ 0 };
        uint32_t m_jointBaseIndex{ 0 };
    };
}
