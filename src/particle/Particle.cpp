#include "Particle.h"

#include "engine/UpdateContext.h"

namespace particle {
    bool Particle::update(const UpdateContext& ctx) noexcept
    {
        m_lifetime -= ctx.m_clock.elapsedSecs;
        if (m_lifetime <= 0) return false;

        m_spriteIndex = (m_spriteIndex + 1) % m_spriteCount;

        m_pos += m_dir * m_velocity * ctx.m_clock.elapsedSecs;
        return true;
    }
}
