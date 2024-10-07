#pragma once

#include <type_traits>
#include <stdint.h>

// channel = metalness, roughness, displacement, ambient - occlusion
enum class TextureType : std::underlying_type_t<std::byte> {
    none,
    diffuse,
    emission,
    specular,
    normal_map,
    dudv_map,
    noise_map,
    noise_2_map,
    metallness_map,
    roughness_map,
    occlusion_map,
    displacement_map,
    opacity_map,
    channel_part_1_map,
    channel_part_2_map,
    channel_part_3_map,
    channel_part_4_map,
    metal_channel_map
};
