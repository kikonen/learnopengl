#include "ParticleDefinition.h"

#include <numbers>

#include "model/NodeType.h"

namespace {
    const float FULL_CIRCLE_RADIANS = std::numbers::pi_v<float> * 2.f;
}

namespace particle {
    glm::vec3 ParticleDefinition::randomPosition(const util::Random& rnd) const
    {
        glm::vec3 p{ 0.f };

        if (m_areaType == particle::AreaType::point) {
            // nothing
        }
        else if (m_areaType == particle::AreaType::sphere_fill) {
            auto radius = m_areaRadius * rnd.rnd();
            glm::vec3 n{
                -0.5f + rnd.rnd(),
                -0.5f + rnd.rnd(),
                -0.5f + rnd.rnd()
            };
            p = glm::normalize(n) * radius;
        }
        else if (m_areaType == particle::AreaType::sphere) {
            auto radius = m_areaRadius;
            glm::vec3 n{
                -0.5f + rnd.rnd(),
                -0.5f + rnd.rnd(),
                -0.5f + rnd.rnd()
            };
            p = glm::normalize(n) * radius;
        }
        else if (m_areaType == particle::AreaType::disc) {
            auto radius = m_areaRadius * rnd.rnd();
            auto radiansY = FULL_CIRCLE_RADIANS * rnd.rnd();
            glm::vec3 n{
                cos(radiansY),
                0.f,
                sin(radiansY),
            };
            p = n * radius;
        }
        else if (m_areaType == particle::AreaType::disc_line) {
            auto radius = m_areaRadius;
            auto radiansY = FULL_CIRCLE_RADIANS * rnd.rnd();
            glm::vec3 n{
                cos(radiansY),
                0.f,
                sin(radiansY),
            };
            p = n * radius;
        }
        else if (m_areaType == particle::AreaType::box_fill) {
            const auto var = m_areaSize;
            p = {
                -var.x + 2.f * var.x * rnd.rnd(),
                -var.y + 2.f * var.y * rnd.rnd(),
                -var.z + 2.f * var.z * rnd.rnd()
            };
        }
        else if (m_areaType == particle::AreaType::box) {
            const auto var = m_areaSize;
            p = {
                -var.x + 2.f * var.x * rnd.rnd(),
                -var.y + 2.f * var.y * rnd.rnd(),
                -var.z + 2.f * var.z * rnd.rnd()
            };

            const auto side = rnd.rnd();
            if (side < 1.f / 3.f) {
                p.x = rnd.rnd() < 0.5f ? -var.x : var.x;
            }
            else  if (side > 2.f * 1.f / 3.f) {
                p.z = rnd.rnd() < 0.5f ? -var.z : var.z;
            }
            else {
                p.y = rnd.rnd() < 0.5f ? -var.y : var.y;
            }
        }
        else if (m_areaType == particle::AreaType::box_line) {
            const auto var = m_areaSize;

            // X/Y vs. X/Z vs. Y/Z plane
            if (rnd.rnd() < 1.f / 3.f) {
                if (rnd.rnd() < .5f) {
                    p.x = -var.x + 2.f * var.x * rnd.rnd();
                    p.y = rnd.rnd() < 0.5f ? -var.y : var.y;
                }
                else {
                    p.x = rnd.rnd() < 0.5f ? -var.x : var.x;
                    p.y = -var.y + 2.f * var.y * rnd.rnd();
                }
                p.z = rnd.rnd() < 0.5f ? -var.z : var.z;
            }
            else if (rnd.rnd() > 2.f / 3.f) {
                if (rnd.rnd() < .5f) {
                    p.x = -var.x + 2.f * var.x * rnd.rnd();
                    p.z = rnd.rnd() < 0.5f ? -var.z : var.z;
                }
                else {
                    p.x = rnd.rnd() < 0.5f ? -var.x : var.x;
                    p.z = -var.z + 2.f * var.z * rnd.rnd();
                }
                p.y = rnd.rnd() < 0.5f ? -var.y : var.y;
            }
            else {
                if (rnd.rnd() < .5f) {
                    p.y = -var.y + 2.f * var.y * rnd.rnd();
                    p.z = rnd.rnd() < 0.5f ? -var.z : var.z;
                }
                else {
                    p.y = rnd.rnd() < 0.5f ? -var.y : var.y;
                    p.z = -var.z + 2.f * var.z * rnd.rnd();
                }
                p.x = rnd.rnd() < 0.5f ? -var.x : var.x;
            }
        }

        glm::vec3 v;
        {
            const auto var = m_areaVariation;
            v = {
                -var.x + 2.f * var.x * rnd.rnd(),
                -var.y + 2.f * var.y * rnd.rnd(),
                -var.z + 2.f * var.z * rnd.rnd()
            };
        }

        return m_areaOffset + p + v;
    }

    glm::vec3 ParticleDefinition::randomDirection(const util::Random& rnd) const
    {
        const auto& normal = m_dir;
        const auto var = m_dirVariation;
        const auto variationX = var * rnd.rnd();
        const auto variationY = var * rnd.rnd();
        const auto variationZ = var * rnd.rnd();
        const float theta = glm::radians(360 * rnd.rnd());

        glm::vec3 n{
            normal.x - var + 2.f * variationX,
            normal.y - var + 2.f * variationY,
            normal.z - var + 2.f * variationZ
        };
        return glm::normalize(n);
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
