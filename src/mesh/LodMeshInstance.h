#pragma once

#include <stdint.h>

#include "util/BufferReference.h"

namespace mesh
{
    struct LodMeshInstance
    {
        util::BufferReference m_rigRef;
        util::BufferReference m_socketRef;
        util::BufferReference m_jointRef;
        uint8_t m_lodMeshIndex;
    };
}
