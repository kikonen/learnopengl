#pragma once

#include <type_traits>

enum class LightType : std::underlying_type_t<std::byte>
{
    none,
    directional,
    point,
    spot
};
