#pragma once

#include "kigl/kigl.h"

namespace backend::gl {
    struct DrawIndirectParameters
    {
        GLuint u_baseIndex;
        GLuint u_drawType;
        GLuint u_drawCount;
    };
}
