#pragma once

#include <map>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;
class RenderContext;

namespace backend {
    struct Lod;
}

namespace render {
    struct BatchKey {
        BatchKey(
            int8_t priority,
            const Program* program,
            const kigl::GLVertexArray* vao,
            const backend::DrawOptions& drawOptions,
            bool forceSolid,
            bool forceWireframe) noexcept;

        bool operator<(const BatchKey& o) const noexcept;

        const Program* m_program{ nullptr };
        const kigl::GLVertexArray* m_vao{ nullptr };

        backend::DrawOptions m_drawOptions;

        const int8_t m_priority;
    };

    struct LodKey {
        const backend::Lod* m_lod;
        uint32_t m_flags;

        bool operator<(const LodKey& o) const noexcept {
            return std::tie(*m_lod, m_flags) < std::tie(*o.m_lod, o.m_flags);
        }
    };

    struct LodEntry {
        uint32_t m_entityIndex;
        uint32_t m_meshIndex;
    };

    struct BatchCommand {
        std::map<LodKey, std::vector<LodEntry>> m_lodInstances;
    };
}
