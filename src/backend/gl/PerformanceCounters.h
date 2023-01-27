#pragma once

#include "ki/GL.h"

namespace backend::gl {
    struct PerformanceCounters
    {
        GLuint u_drawCount{ 0 };
        GLuint u_skipCount{ 0 };
    };
}
