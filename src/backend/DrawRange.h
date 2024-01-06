#pragma once

#include "DrawOptions.h"

class Program;

namespace kigl {
    class GLState;
    struct GLVertexArray;
}

namespace backend {
    struct DrawRange
    {
        kigl::GLState* m_state{ nullptr };
        const Program* m_program{ nullptr };
        const kigl::GLVertexArray* m_vao{ nullptr };
        DrawOptions m_drawOptions;
        bool m_allowBlend : 1 {false};
        bool m_forceWireframe : 1 {false};
    };
}
