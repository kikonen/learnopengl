#pragma once

#include "glm/glm.hpp"

#include "material/Material.h"

#include "ki/size.h"

#include "ParticleSSBO.h"

struct UpdateContext;

namespace particle {
    struct Particle final
    {
        glm::vec3 m_gravity{ 0.f };

        glm::vec3 m_pos{ 0.f };
        glm::vec3 m_velocity{ 0.f };

        float m_lifetime{ 0.f };

        float m_scale{ 1.f };

        float m_spriteSpeed{ 0.f };
        // floor used for render
        float  m_spriteActiveIndex{ 0.f };

        // NOTE KI particle materials required to be max 2^16
        uint16_t m_materialIndex{ 0 };

        uint8_t m_spriteBaseIndex{ 0 };
        uint8_t m_spriteCount{ 1 };

        bool m_alive{ true };

        bool valid() const noexcept
        {
            return m_scale > 0.f && m_lifetime > 0.f && m_materialIndex > 0;
        }

        bool isAlive() const noexcept { return m_alive; }

        void update(const UpdateContext& ctx) noexcept;

        void updateSSBO(ParticleSSBO& ssbo) const noexcept
        {
            ssbo.u_x = m_pos.x;
            ssbo.u_y = m_pos.y;
            ssbo.u_z = m_pos.z;
            //ssbo.u_scale = m_scale;
            //ssbo.u_materialIndex = m_materialIndex;
            //ssbo.u_spriteIndex = m_spriteBaseIndex +
            //    static_cast<uint8_t>(std::max(0.f, m_spriteActiveIndex));

            const auto spriteIndex = m_spriteBaseIndex +
                static_cast<uint8_t>(std::max(0.f, m_spriteActiveIndex));
            ssbo.setMaterialScaleSprite(
                m_materialIndex,
                m_scale,
                spriteIndex);
        }
    };
}
