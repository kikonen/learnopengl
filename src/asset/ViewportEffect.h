#pragma once

enum class ViewportEffect : std::underlying_type_t<std::byte> {
    none = 0,
    invert = 1,
    grayScale = 2,
    sharpen = 3,
    blur = 4,
    edge = 5,
};

