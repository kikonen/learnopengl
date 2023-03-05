#include "Uniform.h"

#include "Program.h"

namespace uniform {

    void Uniform::init(Program* program) {
        if (m_locId == -1) {
            m_locId = program->getUniformLoc(m_name);
        }
    }

    void Subroutine::init(Program* program) {
        int loc2 = program->getUniformSubroutineLoc(m_name, m_shaderType);
        if (m_locId == -1) {
            m_locId = program->getUniformSubroutineLoc(m_name, m_shaderType);
        }
    }

}
