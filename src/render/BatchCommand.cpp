#include "BatchCommand.h"

#include "asset/Program.h"
#include "kigl/GLVertexArray.h"

//#include "render/RenderContext.h"

namespace render {
    BatchKey::BatchKey(
        int8_t priority,
        const Program* program,
        const kigl::GLVertexArray* vao,
        const backend::DrawOptions& drawOptions,
        bool forceSolid,
        bool forceWireframe) noexcept
        : m_program(program),
        m_vao{ vao },
        m_priority( -priority ),
        m_drawOptions{ drawOptions }
    {
        if (forceSolid) {
            m_drawOptions.m_blend = false;
        }
        if (forceWireframe) {
            m_drawOptions.m_wireframe = true;
        }
    }

    //inline bool operator<(const DrawOptions& o) const noexcept {
    //    return std::tie(d.m_blend, d.m_renderBack, m_wireframe, m_type, m_mode) <
    //        std::tie(o.m_blend, o.m_renderBack, o.m_wireframe, o.m_type, o.m_mode);
    //}

    bool BatchKey::operator<(const BatchKey& o) const noexcept
    {
        const auto& d = m_drawOptions;
        const auto& od = o.m_drawOptions;
        return
            std::tie(  m_vao->m_id,  d.m_renderBack,   m_priority,   m_program->m_id,  d.m_blend,  d.m_wireframe,  d.m_type,  d.m_mode) <
            std::tie(o.m_vao->m_id, od.m_renderBack, o.m_priority, o.m_program->m_id, od.m_blend, od.m_wireframe, od.m_type, od.m_mode);
        //return tie ? true : (m_drawOptions < o.m_drawOptions);
    }
}
