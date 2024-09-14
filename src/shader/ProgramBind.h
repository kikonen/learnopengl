#pragma once

#include "Program.h"

class ProgramBind final
{
public:
    ProgramBind(ki::program_id programId) noexcept
        : m_program{ Program::get(programId) }
    {
        if (m_program) m_program->bind();
    };

    ~ProgramBind() noexcept {
        //if (program) program->unbind();
    }

public:
    Program* m_program;
};
