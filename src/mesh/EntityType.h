#pragma once

namespace mesh {
    enum class EntityType : std::underlying_type_t<std::byte> {
        origo,
        container,
        model,
        quad,
        billboard,
        sprite,
        point_sprite,
        terrain,
        skybox,
    };
}
