#pragma once

#include "stdint.h"

namespace backend {
    // Data needed to identify single Lod
    //
    // https://sites.google.com/site/john87connor/indirect-rendering/4-lod-selection-on-gpu?authuser=0
    //
    struct Lod {
        uint32_t m_baseVertex{ 0 };
        uint32_t m_baseIndex{ 0 };
        uint32_t m_indexCount{ 0 };
        // Distance from camera
        float m_distance{ 0.f };

        inline bool operator==(const Lod& o) const noexcept
        {
            return m_baseVertex == o.m_baseVertex &&
                m_baseIndex == o.m_baseIndex;
        }
    };
}
