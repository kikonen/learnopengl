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
        const DrawOptions* m_drawOptions{ nullptr };
        bool m_allowBlend = false;
        bool m_forceWireframe = false;
    };
}
