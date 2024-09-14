#pragma once

#include "DrawOptions.h"

#include "ki/size.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        DrawOptions m_drawOptions;
        ki::vao_id m_vaoId{ 0 };
        ki::program_id m_programId{ 0 };
    };
}
