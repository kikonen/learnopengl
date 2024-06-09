#include "BatchCommand.h"

namespace render {
    BatchKey::BatchKey(
        int programId,
        int vaoId,
        int priority,
        const backend::DrawOptions& drawOptions) noexcept
        : m_programId(programId),
        m_vaoId{ vaoId },
        m_priority( priority ),
        m_mode{ drawOptions.m_mode },
        m_patchVertices{ drawOptions.m_patchVertices },
        m_type{ drawOptions.m_type },
        m_renderBack{ drawOptions.m_renderBack },
        m_wireframe{ drawOptions.m_wireframe },
        m_tessellation{ drawOptions.m_tessellation },
        m_kindBits{ drawOptions.m_kindBits }
    {
    }
}
