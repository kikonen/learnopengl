#pragma once

#include "DrawElementsIndirectCommand.h"
#include "DrawArraysIndirectCommand.h"

namespace backend::gl {
    struct DrawIndirectCommand
    {
        union {
            DrawElementsIndirectCommand element;
            DrawArraysIndirectCommand array;
        };
    };
}
