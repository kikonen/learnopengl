#pragma once

#include "util/BufferReference.h"

namespace animation
{
    struct Rig;
}

namespace mesh
{
    struct RegisteredRig
    {
        const animation::Rig* m_rig{ nullptr };

        util::BufferReference m_rigRef{ 0, 0 };
        util::BufferReference m_socketRef{ 0, 0 };
        util::BufferReference m_jointRef{ 0, 0 };
    };
}
