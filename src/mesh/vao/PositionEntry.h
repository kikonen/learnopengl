#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct PositionEntry {
        PositionEntry()
            : u_x{ 0 }, u_y{ 0 }, u_z{ 0 }
        {}

        PositionEntry(const glm::vec3& v)
            : u_x{ v.x }, u_y{ v.y }, u_z{ v.z }
        {}


        PositionEntry& operator+=(const glm::vec3& v)
        {
            u_x += v.x;
            u_y += v.y;
            u_z += v.z;
            return *this;
        }

        float u_x;
        float u_y;
        float u_z;
    };
#pragma pack(pop)
}
