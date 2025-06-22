#pragma once

#include <glm/glm.hpp>

namespace loader
{
    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        float xStep{ 0.f };
        float yStep{ 0.f };
        float zStep{ 0.f };
    };
}
