#include "BatchCommand.h"

#include "asset/Program.h"
#include "kigl/GLVertexArray.h"

//#include "render/RenderContext.h"

namespace render {
    BatchKey::BatchKey(
        int8_t priority,
        const Program* program,
        const kigl::GLVertexArray* vao,
        const backend::DrawOptions& drawOptions) noexcept
        : m_program(program),
        m_vao{ vao },
        m_priority( -priority ),
        m_drawOptions{ drawOptions }
    {
    }

    bool BatchKey::operator<(const BatchKey& o) const noexcept
    {
        return
            std::tie(  m_priority,   m_program->m_id,   m_vao->m_id, m_drawOptions) <
            std::tie(o.m_priority, o.m_program->m_id, o.m_vao->m_id, o.m_drawOptions);
        //return tie ? true : (m_drawOptions < o.m_drawOptions);
    }
}
