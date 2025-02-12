#include "BatchCommand.h"

#include "kigl/GLVertexArray.h"

#include "shader/Program.h"

#include "mesh/LodMesh.h"

//#include "render/RenderContext.h"

namespace render {
    MultiDrawKey::MultiDrawKey(
        const ki::program_id programId,
        const ki::vao_id vaoId,
        const backend::DrawOptions& drawOptions) noexcept
        : m_programId{ programId },
        m_vaoId{ vaoId },
        m_drawOptions{ drawOptions }
    {
    }

    //inline bool operator<(const DrawOptions& o) const noexcept {
    //    return std::tie(d.m_blend, d.m_renderBack, m_lineMode, m_type, m_mode) <
    //        std::tie(o.m_blend, o.m_renderBack, o.m_lineMode, o.m_type, o.m_mode);
    //}

    bool MultiDrawKey::operator<(const MultiDrawKey& o) const noexcept
    {
        const auto& d = m_drawOptions;
        const auto& od = o.m_drawOptions;
        return
            std::tie(  m_vaoId,   m_programId,  d.m_renderBack,  d.m_kindBits,  d.m_lineMode,  d.m_type,  d.m_mode) <
            std::tie(o.m_vaoId, o.m_programId, od.m_renderBack, od.m_kindBits, od.m_lineMode, od.m_type, od.m_mode);
        //return tie ? true : (m_drawOptions < o.m_drawOptions);
    }

    bool CommandKey::operator<(const CommandKey& o) const noexcept {
        // NOTE KI baseVertex identifies Lod; cannot have multiple in same index
        // - *BUT* indexCount *CAN* very for mesh::TextMesh
        // => relevant if using this as map key
        // NOTE KI material is now per instance (not Entity)
        // => Thus new need to check material here m_lod->m_materialIndex
        //return std::tie(m_baseVertex,  m_indexCount) <
        //    std::tie(o.m_baseVertex, o.m_indexCount);
        return m_baseVertex < o.m_baseVertex;
    }
}
