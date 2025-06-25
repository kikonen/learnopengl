#pragma once

#include <type_traits>

namespace loader
{
    enum class NodeKind : std::underlying_type_t<std::byte> {
        none,
        origo,
        composite,
        marker,
        container,
        model,
        text,
        terrain,
        skybox,
    };
}
