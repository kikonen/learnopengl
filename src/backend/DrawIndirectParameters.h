#pragma once

#include "ki/GL.h"

namespace backend {
    struct DrawIndirectParameters
    {
        // NOTE KI counter *MUST* be first due to GL_PARAMETER_BUFFER 
        GLuint u_counter;
        GLuint u_BaseIndex;
    };
}
