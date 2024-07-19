#pragma once

#include <stdint.h>

namespace mesh {
    struct LodLevel {
        uint8_t m_levelMask{ 0 };

        // Squared Distance upto lod is applied
        float m_distance2{ 0.f };

        void setDistance(float dist) {
            m_distance2 = dist * dist;
        }
    };
}
