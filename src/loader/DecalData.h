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

        uint8_t spriteBaseIndex{ 0 };
        uint8_t spriteCount{ 1 };

        MaterialData materialData;
    };
}
