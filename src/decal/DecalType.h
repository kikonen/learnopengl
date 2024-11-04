#pragma once

#include <type_traits>
#include <stdint.h>

namespace decal {
    enum class DecalType : std::underlying_type_t<std::byte> {
        none,
        normal,
        animated,
    };
}
