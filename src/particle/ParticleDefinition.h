#pragma once

#include <glm/glm.hpp>

namespace particle {
    struct ParticleDefinition {
        glm::vec3 dir{ 0.f };

        float radius = 0.f;
        float velocity = 0.f;
        float velocityVariation = 0.f;
        float size = 1.f;
        float sizeVariation = 1.f;
        float particlesPerSec = 1;
    };
}
