#pragma once

#include <map>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;

namespace backend {
    struct Lod;
}

namespace render {
    struct BatchKey {
        BatchKey(
            int programId,
            int vaoId,
            int priority,
            const backend::DrawOptions& drawOptions) noexcept;

        std::string str() const noexcept
        {
            return fmt::format(
                "<PROGRAM_KEY: id={}, vao={}, pri={}, renderBack={}, wireframe={}>",
                m_programId, m_vaoId, m_priority, m_renderBack, m_wireframe);
        }

        bool operator<(const BatchKey& o) const noexcept {
            // NOTE KI renderBack & wireframe goes into separate render always due to GL state
            // => reduce state changes via sorting
            return std::tie(m_priority,     m_programId,   m_vaoId,   m_mode,   m_patchVertices,   m_type,   m_renderBack,   m_wireframe,   m_tessellation,   m_kindBits) <
                   std::tie(o.m_priority, o.m_programId, o.m_vaoId, o.m_mode, o.m_patchVertices, o.m_type, o.m_renderBack, o.m_wireframe, o.m_tessellation, o.m_kindBits);
        }

        const int m_priority;
        const int m_programId;
        const int m_vaoId;

        const uint8_t m_mode;
        const uint8_t m_patchVertices;
        const backend::DrawOptions::Type m_type : 2;
        const bool m_renderBack : 1;
        const bool m_wireframe : 1;
        const bool m_tessellation : 1;
        const unsigned int m_kindBits : 3;
    };

    struct LodKey {
        const backend::Lod* m_lod;
        bool operator<(const LodKey& o) const noexcept {
            return *m_lod < *o.m_lod;
        }
    };

    struct LodEntry {
        uint32_t m_entityIndex;
        uint32_t m_meshIndex;
    };

    struct BatchCommand {
        const Program* m_program{ nullptr };

        const kigl::GLVertexArray* m_vao{ nullptr };

        std::map<LodKey, std::vector<LodEntry>> m_lodInstances;

        //int m_baseIndex{ 0 };
        int m_instanceCount{ 0 };

        backend::DrawOptions m_drawOptions;
    };
}
