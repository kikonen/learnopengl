#pragma once

#include "DrawElementsIndirectCommand.h"
#include "DrawArraysIndirectCommand.h"

namespace backend {
    struct DrawIndirectCommand
    {
        union {
            DrawElementsIndirectCommand element;
            DrawArraysIndirectCommand array;
        };
    };
}
