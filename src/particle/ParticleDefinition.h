#pragma once

#include <glm/glm.hpp>

#include "util/Random.h"

#include "material/Material.h"

#include "AreaType.h"

class NodeType;

namespace particle {
    class ParticleGenerator;

    struct ParticleDefinition {
        std::shared_ptr<Material> m_material;

        int m_seed{ 0 };

        glm::vec3 m_gravity{ 0.f };

        particle::AreaType m_areaType{ particle::AreaType::none };
        float m_areaRadius{ 0.f };
        glm::vec3 m_areaSize{ 0.f };
        glm::vec3 m_areaOffset{ 0.f };
        glm::vec3 m_areaVariation{ 0.f };

        // angle degrees
        glm::vec3 m_dir{ 0.f };
        float m_dirVariation{ 0.f };

        float m_speed{ 0.f };
        float m_speedVariation{ 0.f };

        // secs
        float m_lifetime{ 0.f };
        float m_lifetimeVariation{ 0.f };

        float m_size{ 1.f };
        float m_sizeVariation{ 0.f };

        // particles per sec
        float m_rate{ 1.f };
        float m_rateVariation{ 0.f };

        int m_spriteBase{ 0 };
        float m_spriteBaseVariation{ 0.f };
        int m_spriteCount{ 1 };
        float m_spriteSpeed{ 0.f };
        float m_spriteSpeedVariation{ 0.f };

    public:
        glm::vec3 randomPosition(const util::Random& rnd) const;
        glm::vec3 randomDirection(const util::Random& rnd) const;
        float randomSpeed(const util::Random& rnd) const;
        float randomLifetime(const util::Random& rnd) const;
        float randomSize(const util::Random& rnd) const;
        float randomSpriteIndex(const util::Random& rnd) const;
        float randomSpriteSpeed(const util::Random& rnd) const;

        static std::unique_ptr<particle::ParticleGenerator> createParticleGenerator(
            const NodeType* type);
    };
}
