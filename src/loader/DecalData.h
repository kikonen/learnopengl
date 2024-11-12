#pragma once

#include "MaterialData.h"

namespace loader {
    struct DecalData {
        std::string name;

        // local rotation (radians) around normal axis
        float rotation{ 0.f };
        float scale{ 1.f };
        float lifetime{ 0.f };
        float spriteSpeed{ 0.f };

        glm::vec2 rotationVariation{ 0.f };
        glm::vec2 scaleVariation{ 0.f };
        glm::vec2 lifetimeVariation{ 0.f };
        glm::vec2 spriteSpeedVariation{ 0.f };

        uint8_t spriteBaseIndex{ 0 };

        MaterialData materialData;
    };
}
