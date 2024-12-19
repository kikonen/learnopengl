#include "ParticleDefinition.h"

namespace particle {
    glm::vec3 ParticleDefinition::randomPosition(const util::Random& rnd) const
    {
        const auto var = m_areaVariation;
        const float variationX = var * rnd.rnd();
        const float variationY = var * rnd.rnd();
        const float variationZ = var * rnd.rnd();

        return {
            var + 2.f * variationX,
            var + 2.f * variationY,
            var + 2.f * variationZ
        };
    }

    glm::vec3 ParticleDefinition::randomDirection(const util::Random& rnd) const
    {
        const auto& normal = m_dir;
        const auto var = m_dirVariation;
        const auto variationX = var * rnd.rnd();
        const auto variationY = var * rnd.rnd();
        const auto variationZ = var * rnd.rnd();
        const float theta = glm::radians(360 * rnd.rnd());

        return {
            normal.x - var + 2.f * variationX,
            normal.y - var + 2.f * variationY,
            normal.z - var + 2.f * variationZ
        };
    }

    float ParticleDefinition::randomSpeed(const util::Random& rnd) const
    {
        const auto speed = m_speed;
        const auto var = m_speedVariation;
        const auto variation = var * rnd.rnd();
        return speed - var + 2.f * variation;
    }

    float ParticleDefinition::randomLifetime(const util::Random& rnd) const
    {
        const auto lifetime = m_lifetime;
        const auto var = m_lifetimeVariation;
        const auto variation = var * rnd.rnd();
        return lifetime - var + 2.f * variation;
    }

    float ParticleDefinition::randomSize(const util::Random& rnd) const
    {
        const auto size = m_size;
        const auto var = m_sizeVariation;
        const auto variation = var * rnd.rnd();
        return size - var + 2.f * variation;
    }

    float ParticleDefinition::randomSpriteIndex(const util::Random& rnd) const
    {
        // NOTE KI start from idx, use full 0..spriteCount range
        const float idx = m_spriteCount * rnd.rnd();

        const auto base = m_spriteBase;
        const auto var = m_spriteBaseVariation;
        const auto variation = var * rnd.rnd();
        return base - var + 2.f * variation;
    }

    float ParticleDefinition::randomSpriteSpeed(const util::Random& rnd) const
    {
        const auto speed = m_spriteSpeed;
        const auto var = m_spriteSpeedVariation;
        const auto variation = var * rnd.rnd();
        return speed - var + 2.f * variation;
    }
}
