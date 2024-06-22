#pragma once

enum class EntityType : std::underlying_type_t<std::byte> {
    none,
    origo,
    marker,
    container,
    model,
    text,
    terrain,
    skybox,
};
