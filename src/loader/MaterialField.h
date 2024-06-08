#pragma once

namespace loader {
    struct MaterialField {
        bool textureSpec = false;

        bool pattern = false;
        bool reflection = false;
        bool refraction = false;
        bool refractionRatio = false;

        bool tilingX = false;
        bool tilingY = false;

        bool spriteCount = false;
        bool spritesX = false;

        bool ns = false;

        bool ka = false;

        bool kd = false;
        bool ks = false;
        bool ke = false;

        bool map_bump_strength = false;

        bool ni = false;
        bool d = false;
        bool illum = false;

        bool layers = false;
        bool layersDepth = false;
        bool parallaxDepth = false;

        bool metal = false;
    };
}
