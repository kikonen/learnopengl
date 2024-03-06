#pragma once

struct MaterialField {
    bool type = false;

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
    bool map_kd = false;

    bool ks = false;
    bool map_ks = false;
    bool ke = false;
    bool map_ke = false;
    bool map_bump = false;
    bool map_bump_strength = false;
    bool ni = false;
    bool d = false;
    bool illum = false;

    bool layers = false;
    bool layersDepth = false;
    bool parallaxDepth = false;

    bool metal = false;

    bool map_dudv = false;
    bool map_height = false;
    bool map_noise = false;

    bool map_roughness = false;
    bool map_metalness = false;
    bool map_occlusion = false;
    bool map_displacement = false;
    bool map_opacity = false;
};
