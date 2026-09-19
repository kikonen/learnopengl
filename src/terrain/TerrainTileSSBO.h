#pragma once

#include <cstdint>

namespace terrain {
    struct TerrainTileSSBO {
        uint32_t u_tileU{ 0 };
        uint32_t u_tileV{ 0 };

        float u_rangeYmin{ 0.f };
        float u_rangeYmax{ 0.f };

        uint32_t u_heightMapTex{ 0 };
    };
}
