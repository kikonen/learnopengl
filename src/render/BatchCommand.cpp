#include "BatchCommand.h"

#include "kigl/GLVertexArray.h"

#include "shader/Program.h"

#include "mesh/LodMesh.h"

//#include "render/RenderContext.h"

namespace render {
    BatchKey::BatchKey(
        int8_t priority,
        const ki::program_id programId,
        GLuint vaoId,
        const backend::DrawOptions& drawOptions,
        bool forceSolid,
        bool forceWireframe) noexcept
        : m_programId{ programId },
        m_vaoId{ vaoId },
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
            std::tie(  m_vaoId,  d.m_renderBack,   m_priority,   m_programId,  d.m_blend,  d.m_wireframe,  d.m_type,  d.m_mode) <
            std::tie(o.m_vaoId, od.m_renderBack, o.m_priority, o.m_programId, od.m_blend, od.m_wireframe, od.m_type, od.m_mode);
        //return tie ? true : (m_drawOptions < o.m_drawOptions);
    }

    LodKey::LodKey(const mesh::LodMesh& lod, uint32_t flags)
        : m_baseVertex{ lod.m_baseVertex },
        m_baseIndex{ lod.m_baseIndex},
        m_indexCount{ lod.m_indexCount },
        m_flags{ flags }
    {
    }

    bool LodKey::operator<(const LodKey& o) const noexcept {
        // NOTE KI baseVertex identifies Lod; cannot have multiple in same index
        // - *BUT* indexCount *CAN* very for mesh::TextMesh 
        // NOTE KI material is now per instance (not Entity)
        // => Thus new need to check material here m_lod->m_materialIndex
        return std::tie(m_baseVertex,  m_indexCount, m_flags) <
            std::tie(o.m_baseVertex, o.m_indexCount, o.m_flags);
    }

}
