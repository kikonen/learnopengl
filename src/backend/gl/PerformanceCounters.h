#pragma once

#include "kigl/kigl.h"

namespace backend::gl {
    struct PerformanceCounters
    {
        GLuint u_drawCount{ 0 };
        GLuint u_skipCount{ 0 };
    };
}
