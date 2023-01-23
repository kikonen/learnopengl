#pragma once

#include "DrawElementsIndirectCommand.h"
#include "DrawArraysIndirectCommand.h"

namespace backend::gl {
    struct CandidateDraw
    {
        GLuint baseInstance;
        union {
            DrawElementsIndirectCommand element;
            DrawArraysIndirectCommand array;
        };
    };
}
