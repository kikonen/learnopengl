#pragma once

namespace mesh {
    enum class EntityType : std::underlying_type_t<std::byte> {
        origo,
        marker,
        container,
        model,
        quad,
        billboard,
        sprite,
        point_sprite,
        text,
        terrain,
        skybox,
    };
}
