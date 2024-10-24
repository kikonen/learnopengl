#pragma once

#include <map>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "backend/DrawOptions.h"


class RenderContext;

namespace mesh {
    struct LodMesh;
}

namespace render {
    struct DrawElement {
        //glm::mat4 m_transform;
        glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        uint32_t m_baseVertex;
        uint32_t m_baseIndex;
        uint32_t m_indexCount;
        uint32_t m_flags;

        float m_distance2;
        uint32_t m_entityIndex;
        uint32_t m_materialIndex;
        int32_t m_socketIndex;

        backend::DrawOptions m_drawOptions;
        ki::vao_id m_vaoId;
        ki::program_id m_programId;
        int8_t m_priority;
    };

    // NOTE KI identifies multi-draw batch
    // => max amount of meshes what can be drawn in same draw call
    // => i.e. vao, program & al.
    // @see DrawBuffer::bindDrawRange
    struct BatchKey {
        backend::DrawOptions m_drawOptions;
        const ki::vao_id m_vaoId;
        const ki::program_id m_programId;
        const int8_t m_priority;

        BatchKey(
            int8_t priority,
            const ki::program_id programId,
            const ki::vao_id vaoId,
            const backend::DrawOptions& drawOptions,
            bool forceSolid,
            bool forceLineMode) noexcept;

        bool operator<(const BatchKey& o) const noexcept;
    };

    struct LodKey {
        const uint32_t m_baseVertex;
        const uint32_t m_baseIndex;
        const uint32_t m_indexCount;
        const uint32_t m_flags;

        LodKey(const mesh::LodMesh& lod, uint32_t flags);

        bool operator<(const LodKey& o) const noexcept;
    };

    struct LodEntry {
        //glm::mat4 m_transform;
        glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        float m_distance2;
        uint32_t m_entityIndex;
        uint32_t m_materialIndex;
        int32_t m_socketIndex;

        LodEntry() {}
        LodEntry(
            const glm::mat4& transform,
            float distance2,
            uint32_t entityIndex,
            uint32_t materialIndex,
            int32_t socketIndex)
            : m_distance2{ distance2 },
            m_entityIndex{ entityIndex },
            m_materialIndex{ materialIndex },
            m_socketIndex{ socketIndex }
        {
            setTransform(transform);
        }

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        inline void setTransform(
            const glm::mat4& mat)
        {
            //u_transformMatrix = mat;
            {
                const auto& c0 = mat[0];
                const auto& c1 = mat[1];
                const auto& c2 = mat[2];
                const auto& c3 = mat[3];

                u_transformMatrixRow0[0] = c0[0];
                u_transformMatrixRow0[1] = c1[0];
                u_transformMatrixRow0[2] = c2[0];
                u_transformMatrixRow0[3] = c3[0];

                u_transformMatrixRow1[0] = c0[1];
                u_transformMatrixRow1[1] = c1[1];
                u_transformMatrixRow1[2] = c2[1];
                u_transformMatrixRow1[3] = c3[1];

                u_transformMatrixRow2[0] = c0[2];
                u_transformMatrixRow2[1] = c1[2];
                u_transformMatrixRow2[2] = c2[2];
                u_transformMatrixRow2[3] = c3[2];
            }
        }
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
