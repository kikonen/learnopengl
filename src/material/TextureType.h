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
    // MRAS: [ambient-occlusion, metalness, roughness, specular]
    // - metalness: 0 = dielectric, 1 = metal
    // - roughness: 0 = smooth/shiny, 1 = rough/matte
    // - occlusion: 0 = fully occluded, 1 = no occ5lusion
    // - specular:  0 = no reflection, 1 = strong reflection
    //
    // (KHR_materials_specular)
    //
    // What Does Specular Control ?
    // Dielectric(non - metal) surfaces : Controls reflectivity strength
    // - 0.5 = standard(4 % reflectance, ~IOR 1.5)
    // - 0.0 = no reflections
    // - 1.0 = strong reflections(glass - like, ~IOR 2.0)
    // Metallic surfaces : Typically ignored(metals use baseColor for F0)
    //
    map_mras,
    map_displacement,
};
