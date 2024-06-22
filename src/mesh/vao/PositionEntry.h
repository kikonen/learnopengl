#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct PositionEntry {
        PositionEntry() {}

        PositionEntry(const glm::vec3& v)
            : x{ v.x }, y{ v.y }, z{ v.z }
        {}


        PositionEntry& operator+=(const glm::vec3& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        float x;
        float y;
        float z;
    };
#pragma pack(pop)
}
