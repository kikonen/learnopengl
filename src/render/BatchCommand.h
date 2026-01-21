#pragma once

#include <map>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "backend/DrawOptions.h"

namespace render {
    class RenderContext;

    struct InstanceEntry {
        uint32_t m_instanceIndex{ 0 };
        InstanceEntry() {}

        InstanceEntry(
            uint32_t instanceIndex)
            : m_instanceIndex{ instanceIndex }
        {
        }
    };

    struct CommandKey {
        const uint32_t m_baseVertex;
        const uint32_t m_baseIndex;

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

        CommandEntry* addCommandEntry(
            uint16_t commandIndex,
            uint32_t indexCount)
        {
            if (m_commands.size() < commandIndex + 1) {
                m_commands.resize(commandIndex + 1);
            }

            CommandEntry& entry = m_commands[commandIndex];
            entry.m_index = commandIndex;
            entry.m_indexCount = indexCount;

            m_dirty = true;

            return &entry;
        }
    };

    struct MultiDrawEntryContainer
    {
        std::vector<MultiDrawEntry> m_pending;

        void clear()
        {
            for (auto& entry : m_pending) {
                entry.clear();
            }
        }

        MultiDrawEntry* addDrawEntry(uint16_t drawIndex)
        {
            if (m_pending.size() < drawIndex + 1) {
                m_pending.resize(drawIndex + 1);
            }

            auto& entry = m_pending[drawIndex];
            entry.m_index = drawIndex;

            return &entry;
        }
    };
}
