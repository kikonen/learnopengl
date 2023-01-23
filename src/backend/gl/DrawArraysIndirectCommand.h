#pragma once

#include "ki/GL.h"

namespace backend::gl {
    struct DrawArraysIndirectCommand
    {
        GLuint vertexCount;
        GLuint instanceCount;
        GLuint firstVertex;
        GLuint baseInstance;
    };
}
