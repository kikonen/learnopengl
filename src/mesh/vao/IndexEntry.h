#pragma once

#include "glm/glm.hpp"

namespace mesh {
    struct IndexEntry {
        IndexEntry(unsigned int a_x, unsigned int a_y, unsigned int a_z)
            : x{ a_x }, y{ a_y }, z{ a_z }
        {}

        IndexEntry(const glm::uvec3& v)
            : x{ v.x }, y{ v.y }, z{ v.z }
        {}

        unsigned int x;
        unsigned int y;
        unsigned int z;
    };
}
