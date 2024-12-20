#pragma once

#include <type_traits>

enum class NodeType : std::underlying_type_t<std::byte> {
    none,
    origo,
    marker,
    container,
    model,
    text,
    terrain,
    skybox,
};
