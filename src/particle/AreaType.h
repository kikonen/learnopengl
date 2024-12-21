#pragma once

#include <type_traits>
#include <stdint.h>

namespace particle {
    enum class AreaType : std::underlying_type_t<std::byte> {
        none,
        point,
        sphere,
        sphere_line,
        disc,
        disc_line,
        box,
        box_line,
    };
}
