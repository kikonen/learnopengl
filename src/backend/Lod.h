#pragma once

#include <tuple>
#include <stdint.h>

namespace backend {
    //
    // Data needed to identify single Lod
    //
    // https://sites.google.com/site/john87connor/indirect-rendering/4-lod-selection-on-gpu?authuser=0
    //
    struct Lod {
        uint32_t m_baseVertex{ 0 };
        uint32_t m_baseIndex{ 0 };
        uint32_t m_indexCount{ 0 };

        // Distance upto lod is applied
        float m_distance{ 0.f };

        // Squared Distance upto lod is applied
        float m_distance2{ 0.f };

        uint32_t m_materialIndex{ 0 };

        void setDistance(float dist) {
            m_distance = dist;
            m_distance2 = dist * dist;
        }

        inline bool operator==(const Lod& o) const noexcept
        {
            // NOTE KI there cannot be different index count mesh in same base
            return m_baseVertex == o.m_baseVertex &&
                m_baseIndex == o.m_baseIndex &&
                m_materialIndex == o.m_materialIndex;
        }

        inline bool operator<(const Lod& o) const noexcept {
            // NOTE KI there cannot be different index count mesh in same base
            return std::tie(m_baseVertex, m_baseIndex, m_materialIndex) <
                std::tie(o.m_baseVertex, o.m_baseIndex, o.m_materialIndex);
        }
    };
}
