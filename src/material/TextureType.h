#pragma once

#include <type_traits>
#include <stdint.h>

// metal_channel = metalness, roughness, displacement, ambient - occlusion
enum class TextureType : std::underlying_type_t<std::byte> {
    none,
    diffuse,
    emission,
    specular,
    map_normal,
    map_dudv,
    map_noise,
    map_noise_2,
    map_metallness,
    map_roughness,
    map_occlusion,
    map_displacement,
    map_opacity,
    map_channel_part_1,
    map_channel_part_2,
    map_channel_part_3,
    map_channel_part_4,
    map_metal_channel
};
