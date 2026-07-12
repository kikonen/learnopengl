#pragma once

#include <string>

#include "ki/size.h"

#include "util/Ref.h"

#include "Decal.h"

struct Material;

namespace decal
{
    struct DecalDefinition {
        ki::decal_id m_sid;

        // local rotation (radians) around normal axis
        float m_rotation{ 0.f };
        float m_scale{ 1.f };

        float m_lifetime{ 0.f };

        ki::material_index m_materialIndex{ 0 };

        float m_spriteSpeed{ 0.f };
        uint8_t m_spriteBaseIndex{ 0 };
        uint8_t m_spriteCount{ 1 };

        glm::vec2 m_rotationVariation{ 0.f };
        glm::vec2 m_scaleVariation{ 0.f };
        glm::vec2 m_lifetimeVariation{ 0.f };
        glm::vec2 m_spriteSpeedVariation{ 0.f };

        util::Ref<Material> m_material;

        bool m_static : 1 { true };

        bool isValid() const noexcept {
            return m_sid != 0;
        }

        void setMaterial(const util::Ref<Material>& src) noexcept;

        Decal createForHit(
            pool::NodeHandle parent,
            const glm::vec3& hitPos,
            const glm::vec3& hitNormal);
    };
}
