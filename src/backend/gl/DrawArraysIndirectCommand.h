#pragma once

#include "kigl/kigl.h"

namespace backend::gl {
    struct DrawArraysIndirectCommand
    {
        GLuint vertexCount;
        GLuint instanceCount;
        GLuint firstVertex;
        GLuint baseInstance;
    };
}
