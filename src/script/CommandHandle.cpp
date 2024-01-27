#include "CommandHandle.h"

#include <vector>
#include <mutex>
#include <map>

#include "pool/Pool.hpp"
#include "pool/IdGenerator.h"

#include "CommandEntry.h"

namespace {
    IdGenerator<script::command_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 100000 };

    pool::Pool<script::CommandEntry> s_pool{ MAX_POOL_SIZE };

    std::map<script::command_id, uint32_t> m_IdToIndex;
}

namespace script {
    CommandHandle CommandHandle::NULL_HANDLE{ 0, 0 };

    CommandHandle& CommandHandle::operator=(const CommandEntry* entry) noexcept
    {
        if (entry) {
            m_handleIndex = entry->m_handleIndex;
            m_id = entry->m_id;
        }
        return *this;
    }

    void CommandHandle::release() noexcept
    {
        if (!m_handleIndex) return;

        auto& entry = s_pool.getEntry(m_handleIndex);
        if (entry.m_data.m_id && entry.m_data.m_id == m_id) {
            // TODO KI release
            entry.m_data.m_id = 0;
            entry.m_data.m_handleIndex = 0;

            m_id = 0;
            m_handleIndex = 0;
        }
    }

    CommandEntry* CommandHandle::toCommand() const noexcept
    {
        if (!m_handleIndex) return nullptr;

        auto& entry = s_pool.getEntry(m_handleIndex);
        if (entry.m_data.m_id && entry.m_data.m_id == m_id) {
            return &entry.m_data;
        }

        // TODO KI invalidated; clear iteslf
        //clear();

        return nullptr;
    }

    CommandHandle CommandHandle::allocate(script::command_id id) noexcept
    {
        if (!id) return NULL_HANDLE;

        auto handleIndex = s_pool.allocate();
        auto& entry = s_pool.getEntry(handleIndex);

        entry.m_data.m_id = id;
        entry.m_data.m_handleIndex = handleIndex;

        m_IdToIndex.insert({ id, handleIndex });

        return { handleIndex, id };
    }

    CommandHandle CommandHandle::toHandle(script::command_id id) noexcept
    {
        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return {};
        return { it->second, id };
    }

    CommandEntry* CommandHandle::toCommand(script::command_id id) noexcept
    {
        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return nullptr;
        CommandHandle handle{ it->second, id };
        return handle.toCommand();
    }

    void CommandHandle::clear() noexcept
    {
        s_pool.clear();
    }

    script::command_id CommandHandle::nextId()
    {
        return ID_GENERATOR.nextId();
    }
}
