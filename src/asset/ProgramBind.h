#pragma once

#include "kigl/GLState.h"

#include "asset/Program.h"

class ProgramBind final
{
public:
    ProgramBind(Program* program, GLState& state) noexcept
        : program(program)
    {
        if (program) program->bind(state);
    };

    ~ProgramBind() noexcept {
        //if (program) program->unbind();
    }

public:
    Program* program;
};
