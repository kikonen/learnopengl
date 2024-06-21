#pragma once

#include "DrawOptions.h"

class Program;

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        const kigl::GLVertexArray* m_vao{ nullptr };
        const Program* m_program{ nullptr };
        DrawOptions m_drawOptions;
    };
}
