#pragma once

#include <type_traits>

// NOTE KI MUST match shader/viewport.fs constants
//
// #define EFF_NONE 0
// #define EFF_INVERT 1
// #define EFF_GRAY_SCALE 2
// #define EFF_SHARPEN 3
// #define EFF_BLUR 4
// #define EFF_EDGE 5
//
enum class ViewportEffect : std::underlying_type_t<std::byte> {
    none = 0,
    invert = 1,
    gray_scale = 2,
    sharpen = 3,
    blur = 4,
    edge = 5,
};
