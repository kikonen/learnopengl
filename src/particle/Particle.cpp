#include "Particle.h"

#include "engine/UpdateContext.h"

namespace particle {
    bool Particle::update(const UpdateContext& ctx)
    {
        m_lifetime -= ctx.m_clock.elapsedSecs;
        if (m_lifetime <= 0) return false;

        m_index = (m_index + 1) % m_maxIndex;

        m_pos += m_dir * m_velocity * ctx.m_clock.elapsedSecs;
        return true;
    }
}
