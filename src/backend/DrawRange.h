#pragma once

#include "DrawOptions.h"

#include "ki/size.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        GLuint m_vaoId{ 0 };
        ki::program_id m_programId{ 0 };
        DrawOptions m_drawOptions;
    };
}
