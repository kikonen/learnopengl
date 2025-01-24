#pragma once

#include <type_traits>
#include <stdint.h>

enum class TextureType : std::underlying_type_t<std::byte> {
    none,
    diffuse,
    emission,
    // NOTE KI specular is obsolete in current design
    specular,
    map_normal,
    map_dudv,
    map_noise,
    map_noise_2,
    map_opacity,
    map_custom_1,
    // BUILD assets
    // MRAO: [metalness, roughness, ambient-occlusion]
    map_mrao,
    map_displacement,
};
