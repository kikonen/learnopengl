#pragma once

#include <glm/glm.hpp>

namespace loader
{
    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        double xStep{ 0.f };
        double yStep{ 0.f };
        double zStep{ 0.f };
    };

    struct Tiling {
        int tile_size{ 100 };
        glm::uvec3 tiles{ 1 };
        float height_scale{ 32 };
        float horizontal_scale{ 1 };
        glm::vec2 vertical_range{ 0, 32 };
    };
}
