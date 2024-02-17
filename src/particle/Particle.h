#pragma once

#include "glm/glm.hpp"

#include "asset/Material.h"

#include "ki/size.h"

#include "ParticleSSBO.h"

struct UpdateContext;

namespace particle {
    struct Particle final
    {
        glm::vec3 m_pos{ 0.f };
        glm::vec3 m_dir{ 0.f };
        float m_velocity{ 0.f };
        float m_lifetime{ 0.f };

        float m_scale{ 1.f };

        GLint m_materialIndex{ 0 };

        bool update(const UpdateContext& ctx);

        const ParticleSSBO toSSBO() const noexcept
        {
            return {
                m_pos,
                m_scale,
                m_materialIndex,
            };
        }
    };
}
