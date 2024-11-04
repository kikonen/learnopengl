#pragma once

#include <string>

#include "ki/sid.h"
#include "ki/size.h"

namespace decal
{
    struct DecalDefinition {
        ki::StringID m_sid;

        // local rotation (radians) around normal axis
        float m_rotation{ 0.f };
        float m_scale{ 1.f };

        float m_lifetime{ 0.f };

        ki::material_index m_materialIndex{ 0 };

        float m_spriteSpeed{ 0.f };

        uint8_t m_spriteBaseIndex{ 0 };
        uint8_t m_spriteCount{ 1 };

        operator bool() const noexcept {
            return m_sid;
        }
    };
}
