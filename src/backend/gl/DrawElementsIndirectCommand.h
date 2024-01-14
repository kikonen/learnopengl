#pragma once

#include "kigl/kigl.h"

namespace backend::gl {
    struct DrawElementsIndirectCommand
    {
        GLuint u_count;
        GLuint u_instanceCount;
        GLuint u_firstIndex;
        GLuint u_baseVertex;
        GLuint u_baseInstance;
    };
}
