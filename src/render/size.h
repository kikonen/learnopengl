#pragma once

namespace render {
    constexpr unsigned int KIND_SOLID{ 1 << 0 };
    constexpr unsigned int KIND_ALPHA{ 1 << 2 };
    constexpr unsigned int KIND_BLEND{ 1 << 3 };
    constexpr unsigned int KIND_ALL{ KIND_SOLID | KIND_ALPHA | KIND_BLEND };
}
