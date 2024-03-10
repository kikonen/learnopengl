#pragma once

#include "kigl/kigl.h"

namespace terrain {
    struct TerrainTileSSBO {
        GLuint u_tileU{ 0 };
        GLuint u_tileV{ 0 };

        float u_rangeYmin{ 0.f };
        float u_rangeYmax{ 0.f };

        GLuint64 u_heightMapTex{ 0 };
    };
}
