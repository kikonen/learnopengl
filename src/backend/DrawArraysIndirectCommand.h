#pragma once

#include "ki/GL.h"

namespace backend {
    struct DrawArraysIndirectCommand
    {
        GLuint vertexCount;
        GLuint instanceCount;
        GLuint firstVertex;
        GLuint baseInstance;
    };
}
