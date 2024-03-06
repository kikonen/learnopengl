#pragma once

#include <string>

#include <glm/glm.hpp>

#include "particle/ParticleDefinition.h"

namespace loader {
    struct ParticleData {
        bool enabled{ false };

        std::string name;

        glm::vec3 dir{ 0.f };

        float radius{ 0.f };
        float velocity{ 0.f };
        float velocityVariation{ 0.f };
        float size{ 1.f };
        float sizeVariation{ 1.f };
        float particlesPerSes{ 1.f };

        std::string materialName;
    };
}
