#pragma once

#include "DrawOptions.h"

class GLState;
class Shader;
struct GLVertexArray;

namespace backend {
    struct DrawRange
    {
        GLState* state{ nullptr };
        const Shader* shader{ nullptr };
        const GLVertexArray* vao{ nullptr };
        const DrawOptions* drawOptions{ nullptr };
        bool useBlend = false;
    };
}
