#pragma once

#include <stdint.h>

#include "TerrainTileSSBO.h"

class ImageTexture;

namespace terrain {
    struct TerrainTileInfo {
        int m_tileU{ 0 };
        int m_tileV{ 0 };

        float u_rangeYmin{ 0.f };
        float u_rangeYmax{ 0.f };

        ImageTexture* m_heightMapTex{ nullptr };
        GLuint64 m_heightMapTexHandle{ 0 };

        uint32_t m_registeredIndex{ 0 };

        void updateSSBO(TerrainTileSSBO& ssbo) const noexcept
        {
            ssbo.u_tileU = m_tileU;
            ssbo.u_tileV = m_tileV;

            ssbo.u_rangeYmin = u_rangeYmin;
            ssbo.u_rangeYmax = u_rangeYmax;

            ssbo.u_heightMapTex = m_heightMapTexHandle;
        }
    };
}
