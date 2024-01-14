#pragma once

#include "kigl/kigl.h"

namespace backend::gl {
    struct DrawArraysIndirectCommand
    {
        GLuint u_vertexCount;
        GLuint u_instanceCount;
        GLuint u_firstVertex;
        GLuint u_baseInstance;
    };
}
