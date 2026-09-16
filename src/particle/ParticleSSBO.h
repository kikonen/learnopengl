#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

namespace particle {
    //
    // SSBO entry
    //
#pragma pack(push, 1)
    struct ParticleSSBO {
        // xyz = pos, a = scale
        float u_x;
        float u_y;
        float u_z;

        //uint32_t u_materialIndex;
        //float u_scale;
        //uint32_t u_spriteIndex;
        uint32_t u_msp;

        void setMaterialScaleSprite(
            uint32_t materialIndex,
            float scale,
            uint32_t spriteIndex)
        {
            // range = 0..5
            constexpr float MAX_SCALE = 5.f;
            uint32_t mappedScale = static_cast<uint32_t>((std::min(MAX_SCALE, scale) / MAX_SCALE) * 0xff);
            u_msp = (materialIndex << 16)
                | (mappedScale << 8)
                | std::min((uint32_t)0xff, spriteIndex);
        }
    };
#pragma pack(pop)
}
