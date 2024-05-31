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

        int m_baseIndex{ 0 };
        int m_instanceCount{ 0 };

        backend::DrawOptions m_drawOptions;
    };
}
