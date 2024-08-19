#include "Particle.h"

//#include "util/debug.h"

#include "engine/UpdateContext.h"

namespace particle {
    bool Particle::update(const UpdateContext& ctx) noexcept
    {
        const auto dt = ctx.m_clock.elapsedSecs;

        m_lifetime -= dt;
        if (m_lifetime <= 0) return false;

        m_spriteActiveIndex += std::fmodf(dt * m_spriteSpeed, (float)m_spriteCount);

        //KI_INFO_OUT(fmt::format(
        //    "index={}, active={}, speed={}, count={}",
        //    m_spriteBaseIndex + static_cast<uint8_t>(m_spriteActiveIndex), m_spriteActiveIndex, m_spriteSpeed, m_spriteCount));

        m_pos += m_dir * m_velocity * dt;
        return true;
    }
}
