#pragma once

#include "ki/GL.h"

namespace backend::gl {
    struct DrawIndirectParameters
    {
        GLuint u_baseIndex;
        GLuint u_drawType;
    };
}
