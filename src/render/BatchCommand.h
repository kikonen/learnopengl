#pragma once

#include <map>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "backend/DrawOptions.h"


class RenderContext;

namespace render {
    struct InstanceEntry {
        //glm::mat4 m_transform;
        glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        float m_distance2;
        uint32_t m_entityIndex;
        uint32_t m_materialIndex;
        int32_t m_socketIndex;
        uint32_t m_data;

        InstanceEntry() {}

        InstanceEntry(
            const glm::mat4& transform,
            float distance2,
            uint32_t entityIndex,
            uint32_t materialIndex,
            int32_t socketIndex,
            uint32_t data)
            : m_distance2{ distance2 },
            m_entityIndex{ entityIndex },
            m_materialIndex{ materialIndex },
            m_socketIndex{ socketIndex },
            m_data{ data }
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

    struct CommandKey {
        uint32_t m_baseVertex;
        uint32_t m_baseIndex;

        CommandKey(
            uint32_t baseVertex,
            uint32_t baseIndex)
            : m_baseVertex{ baseVertex },
            m_baseIndex{ baseIndex }
        {
        }

        bool operator<(const CommandKey& o) const noexcept;

        bool operator==(const CommandKey& o) const
        {
            return m_baseVertex == o.m_baseVertex &&
                m_baseIndex == o.m_baseIndex;
        }
    };

    // NOTE KI identifies multi-draw batch
    // => max amount of meshes what can be drawn in same draw call
    // => i.e. vao, program & al.
    // @see DrawBuffer::bindDrawRange
    struct MultiDrawKey {
        ki::vao_id m_vaoId;
        ki::program_id m_programId;
        backend::DrawOptions m_drawOptions;

        MultiDrawKey(
            const ki::program_id programId,
            const ki::vao_id vaoId,
            const backend::DrawOptions& drawOptions) noexcept;

        bool operator<(const MultiDrawKey& o) const noexcept;

        bool operator==(const MultiDrawKey& o) const
        {
            return m_vaoId == o.m_vaoId &&
                m_programId == o.m_programId &&
                m_drawOptions.isSameMultiDraw(o.m_drawOptions);
        }
    };

    struct CommandEntry {
        InstanceEntry* m_instances{ nullptr };
        uint32_t m_reservedSize{ 0 };

        // NOTE KI base index into InstanceSSBO
        uint32_t m_baseIndex{ 0 };

        uint32_t m_instanceCount{ 0 };

        // NOTE KI variant in dyanmic text mesh case
        // => Store separately from commandKey
        uint32_t m_indexCount{ 0 };

        int16_t m_index{ 0 };

        void clear()
        {
            m_baseIndex = 0;
            m_instanceCount = 0;
        }

        void reserve(size_t size) {
            const uint32_t newSize = std::max(m_reservedSize, static_cast<uint32_t>(size));
            if (newSize <= m_reservedSize) return;

            InstanceEntry* old = m_instances;

            m_instances = new InstanceEntry[size];
            m_reservedSize = newSize;

            if (old) {
                if (m_instanceCount > 0) {
                    memcpy(m_instances, old, sizeof(InstanceEntry) * m_instanceCount);
                }
                delete old;
            }
        }

        inline bool empty() const noexcept
        {
            return m_instanceCount == 0;
        }

        inline void addInstance(InstanceEntry instance)
        {
            if (m_instanceCount + 1 > m_reservedSize) {
                reserve(std::max(
                    static_cast<uint32_t>(m_reservedSize * 1.25f),
                    m_instanceCount + 1));
            }
            m_instances[m_instanceCount++] = instance;
        }
    };

    struct MultiDrawEntry {
        int16_t m_index{ 0 };
        bool m_dirty{ false };

        std::vector<CommandEntry> m_commands;

        void clear() {
            m_dirty = false;
            for (auto& command : m_commands) {
                command.clear();
            }
        }

        inline bool empty() const noexcept
        {
            return !m_dirty;
        }
    };
}
