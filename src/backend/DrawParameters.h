#pragma once

#include "ki/GL.h"

namespace backend {
    struct DrawParameters
    {
        // NOTE KI counter *MUST* be first due to GL_PARAMETER_BUFFER 
        GLuint u_counter;
        GLuint u_offset;
    };
}
