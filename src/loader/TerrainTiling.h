#pragma once

#include <glm/glm.hpp>

namespace loader
{
    struct TerrainTiling {
        int tile_size{ 100 };
        glm::uvec3 tiles{ 1 };
        float height_scale{ 32 };
        float horizontal_scale{ 1 };
        glm::vec2 vertical_range{ 0, 32 };
    };
}
