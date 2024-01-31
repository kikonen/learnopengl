#pragma once

#include "DrawOptions.h"

class Program;

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        const Program* m_program{ nullptr };
        const kigl::GLVertexArray* m_vao{ nullptr };
        DrawOptions m_drawOptions;
        bool m_allowBlend : 1 {false};
        bool m_forceWireframe : 1 {false};
    };
}
