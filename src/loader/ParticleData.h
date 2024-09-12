#pragma once

#include <string>

#include <glm/glm.hpp>

#include "particle/ParticleDefinition.h"
#include "MaterialData.h"

namespace loader {
    struct ParticleData {
        bool enabled{ false };

        std::string name;

        // angle degrees
        glm::vec3 dir{ 0.f };
        float dirVariation{ 0.f };

        // secs
        float lifetime{ 0.f };

        float radius{ 0.f };

        float velocity{ 0.f };
        float velocityVariation{ 0.f };

        float size{ 1.f };
        float sizeVariation{ 0.f };

        // particles per sec
        float rate{ 1000.f };
        float rateVariation{ 0.f };

        MaterialData materialData;
    };
}
