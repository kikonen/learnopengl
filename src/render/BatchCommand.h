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
    // NOTE KI identifies multi-draw batch
    // => max amount of meshes what can be drawn in same draw call
    // => i.e. vao, program & al.
    // @see DrawBuffer::bindDrawRange
    struct BatchKey {
        BatchKey(
            int8_t priority,
            const Program* program,
            const kigl::GLVertexArray* vao,
            const backend::DrawOptions& drawOptions,
            bool forceSolid,
            bool forceWireframe) noexcept;

        bool operator<(const BatchKey& o) const noexcept;

        const Program* m_program;
        const kigl::GLVertexArray* m_vao;

        backend::DrawOptions m_drawOptions;

        const int8_t m_priority;
    };

    struct LodKey {
        const uint32_t m_baseVertex;
        const uint32_t m_baseIndex;
        const uint32_t m_indexCount;
        const uint32_t m_flags;

        LodKey(const backend::Lod& lod, uint32_t flags);

        bool operator<(const LodKey& o) const noexcept;
    };

    struct LodEntry {
        const uint32_t m_entityIndex;
        const uint32_t m_meshIndex;
        const uint32_t m_materialIndex;
        const uint32_t m_socketIndex;
    };

    struct BatchCommand {
        std::map<LodKey, std::vector<LodEntry>> m_lodInstances;
        std::map<LodKey, uint32_t> m_baseIndeces;

        uint32_t getBaseIndex(const LodKey& lodKey) const noexcept
        {
            const auto& it = m_baseIndeces.find(lodKey);
            return it != m_baseIndeces.end() ? it->second : 0;
        }
    };
}
