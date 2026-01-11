#include "Particle.h"

//#include "util/debug.h"

#include "engine/UpdateContext.h"

namespace particle {
    void Particle::update(const UpdateContext& ctx) noexcept
    {
        const auto dt = ctx.getClock().elapsedSecs;

        m_lifetime -= dt;
        if (m_lifetime <= 0) {
            m_alive = false;
            return;
        }

        if (m_spriteSpeed >= 0) {
            m_spriteActiveIndex = std::fmodf(m_spriteActiveIndex + dt * m_spriteSpeed, (float)m_spriteCount);
        }
        else {
            m_spriteActiveIndex += dt * m_spriteSpeed;
            if (m_spriteActiveIndex < 0) {
                m_spriteActiveIndex = ((float)m_spriteCount) + m_spriteActiveIndex;
            }
        }

        m_velocity += m_gravity * dt;
        m_pos += m_velocity * dt;
    }
}
