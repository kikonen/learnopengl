#pragma once

#include <glm/glm.hpp>

namespace particle {
    struct ParticleDefinition {
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
        float rate{ 1.f };
        float rateVariation{ 0.f };
    };
}
