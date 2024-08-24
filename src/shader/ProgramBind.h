#pragma once

#include "Program.h"

class ProgramBind final
{
public:
    ProgramBind(Program* program) noexcept
        : program(program)
    {
        if (program) program->bind();
    };

    ~ProgramBind() noexcept {
        //if (program) program->unbind();
    }

public:
    Program* program;
};
