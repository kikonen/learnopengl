#pragma once

enum class NodeType : std::underlying_type_t<std::byte> {
    none,
    origo,
    marker,
    container,
    model,
    text,
    primitive,
    terrain,
    skybox,
};
