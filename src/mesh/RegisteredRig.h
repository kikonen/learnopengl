#pragma once

#include "util/Ref.h"

#include "util/BufferReference.h"

namespace animation
{
    struct Rig;
}

namespace mesh
{
    struct RegisteredRig
    {
        util::Ref<animation::Rig> m_rig{ nullptr };
        util::Ref<animation::JointContainer> m_jointContainer{ nullptr };

        util::BufferReference m_rigRef{ 0, 0 };
        util::BufferReference m_socketRef{ 0, 0 };
        util::BufferReference m_jointRef{ 0, 0 };

        // true if this entry owns the rig/socket registration
        // false for accessory entries that share the rig but have their own joint palette
        bool m_ownsRig{ true };
    };
}
