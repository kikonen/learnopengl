#pragma once

#include <type_traits>
#include <stdint.h>

namespace particle {
    enum class AreaType : std::underlying_type_t<std::byte> {
        none,
        point,
        sphere_fill,
        sphere,
        disc,
        disc_line,
        box,
        box_fill,
        box_line,
    };
}
