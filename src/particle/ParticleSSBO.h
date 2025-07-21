#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"

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

        //GLuint u_materialIndex;
        //float u_scale;
        //GLuint u_spriteIndex;
        GLuint u_msp;

        void setMaterialScaleSprite(
            GLuint materialIndex,
            float scale,
            GLuint spriteIndex)
        {
            // range = 0..5
            constexpr float MAX_SCALE = 5.f;
            GLuint mappedScale = static_cast<GLuint>((std::min(MAX_SCALE, scale) / MAX_SCALE) * 0xff);
            u_msp = (materialIndex << 16)
                | (mappedScale << 8)
                | std::min((GLuint)0xff, spriteIndex);
        }
    };
#pragma pack(pop)
}
