#pragma once

#include "DrawOptions.h"

class GLState;
class Shader;
struct GLVertexArray;

namespace backend {
    struct DrawRange
    {
        GLState* m_state{ nullptr };
        const Shader* m_shader{ nullptr };
        const GLVertexArray* m_vao{ nullptr };
        const DrawOptions* m_drawOptions{ nullptr };
        bool m_useBlend = false;
    };
}
