#pragma once

#include "DrawOptions.h"

class GLState;
class Program;
struct GLVertexArray;

namespace backend {
    struct DrawRange
    {
        GLState* m_state{ nullptr };
        const Program* m_program{ nullptr };
        const GLVertexArray* m_vao{ nullptr };
        const DrawOptions* m_drawOptions{ nullptr };
        bool m_allowBlend = false;
        bool m_forceWireframe = false;
    };
}
