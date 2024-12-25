#pragma once

#include <type_traits>
#include <stdint.h>

namespace text {
    enum class Align : std::underlying_type_t<std::byte> {
        none,
        left,
        right,
        center,
        top,
        bottom,
    };
}
