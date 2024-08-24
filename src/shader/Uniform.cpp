#include "Uniform.h"

#include "Program.h"

namespace uniform {

    void Uniform::init(Program* program) {
        if (m_locId == -1) {
            m_locId = program->getUniformLoc(m_name);
        }
    }

    void Subroutine::init(Program* program) {
        if (m_indeces) return;

        if (m_locId == -1) {
            m_locId = program->getUniformSubroutineLoc(m_name, m_shaderType);
        }

        // NOTE KI *validate* effect, i.e. debug_depth renderer does not have u_effect
        //m_locId = program->getUniformSubroutineLoc(m_name, m_shaderType);

        if (m_locId != -1) {
            GLsizei n = m_locId + 1;
            //glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);

            m_indeces = new GLuint[n];
            for (int i = 0; i < n; i++) {
                m_indeces[i] = 0;
            }
        }
    }

    void Subroutine::set(GLuint routineIndex, bool force) noexcept {
        if (m_locId != -1 && (force || m_unassigned || routineIndex != m_lastValue)) {
            m_indeces[m_locId] = routineIndex;
            glUniformSubroutinesuiv(m_shaderType, m_locId + 1, m_indeces);
            m_lastValue = routineIndex;
            //m_unassigned = force;
        }
    }

}
