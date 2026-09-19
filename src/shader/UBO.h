#pragma once

#include <cstdint>

// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs


constexpr uint32_t UBO_MATRICES = 0;
constexpr uint32_t UBO_DATA = 1;
constexpr uint32_t UBO_CLIP_PLANES = 2;
constexpr uint32_t UBO_LIGHTS = 3;
constexpr uint32_t UBO_CAMERA = 4;
//constexpr uint32_t UBO_MATERIALS = 4;
//constexpr uint32_t UBO_TEXTURES = 5;
constexpr uint32_t UBO_SHADOW = 5;
constexpr uint32_t UBO_BUFFER_INFO = 6;
constexpr uint32_t UBO_DEBUG = 7;
