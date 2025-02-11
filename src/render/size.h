#pragma once

namespace render {
    inline constexpr unsigned int KIND_NONE{ 0 };
    inline constexpr unsigned int KIND_SOLID{ 1 << 0 };
    inline constexpr unsigned int KIND_ALPHA{ 1 << 1 };
    inline constexpr unsigned int KIND_BLEND{ 1 << 2 };
    inline constexpr unsigned int KIND_ALL{ KIND_SOLID | KIND_ALPHA | KIND_BLEND };
}
