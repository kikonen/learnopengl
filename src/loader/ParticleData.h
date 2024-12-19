#pragma once

#include <string>

#include <glm/glm.hpp>

#include "particle/AreaType.h"
#include "particle/ParticleDefinition.h"
#include "MaterialData.h"

namespace loader {
    struct ParticleData {
        bool explicitEnabled{ false };
        bool enabled{ false };

        std::string name;

        int seed{ 0 };

        // angle degrees
        glm::vec3 dir{ 0.f };
        float dirVariation{ 0.f };

        // secs
        float lifetime{ 0.f };
        float lifetimeVariation{ 0.f };

        particle::AreaType areaType{ particle::AreaType::point };
        float areaRadius{ 0.f };
        glm::vec3 areaSize{ 0.f };
        float areaVariation{ 0.f };

        float velocity{ 0.f };
        float velocityVariation{ 0.f };

        float size{ 1.f };
        float sizeVariation{ 0.f };

        // particles per sec
        float rate{ 1.f };
        float rateVariation{ 0.f };

        int spriteBase{ 0 };
        float spriteBaseVariation{ 0.f };
        int spriteCount{ 1 };
        float spriteSpeed{ 0.f };
        float spriteSpeedVariation{ 0.f };

        MaterialData materialData;
    };
}
