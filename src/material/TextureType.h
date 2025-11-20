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
    //map_opacity,
    map_custom_1,
    // BUILD assets
    // MRAO: [ambient-occlusion, metalness, roughness, opacity]
    // - metalness: 0 = dielectric, 1 = metal
    // - roughness: 0 = smooth/shiny, 1 = rough/matte
    // - occlusion: 0 = fully occluded, 1 = no occlusion
    // - opacity:   0 = transparent, 1 = opaque
    map_mrao,
    map_displacement,
};
