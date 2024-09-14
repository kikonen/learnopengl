#pragma once

#include "DrawOptions.h"

#include "ki/size.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        const kigl::GLVertexArray* m_vao{ nullptr };
        ki::program_id m_programId{ 0 };
        DrawOptions m_drawOptions;
    };
}
