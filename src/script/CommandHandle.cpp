#include "CommandHandle.h"

#include <vector>
#include <mutex>
#include <unordered_map>

#include "pool/Pool_impl.h"
#include "pool/IdGenerator.h"

#include "CommandEntry.h"

namespace {
    IdGenerator<script::command_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 200000 };

    //std::mutex m_lock;
    pool::Pool<script::CommandEntry> s_pool{ MAX_POOL_SIZE };

    //std::unordered_map<script::command_id, uint32_t> m_IdToIndex;
}

namespace script {
    CommandHandle CommandHandle::NULL_HANDLE{ 0, 0 };

    CommandHandle::~CommandHandle()
    {
    }

    void CommandHandle::release() noexcept
    {
        if (!m_handleIndex) return;

        //std::lock_guard lock(m_lock);

        auto* entry = s_pool.getEntry(m_handleIndex);
        if (!entry) return;

        if (entry->m_data.m_id && entry->m_data.m_id == m_id) {
            s_pool.release(m_handleIndex);

            //const auto& it = m_IdToIndex.find(m_id);
            //if (it != m_IdToIndex.end()) {
            //    m_IdToIndex.erase(it);
            //}

            m_id = 0;
            m_handleIndex = 0;
        }
    }

    CommandEntry* CommandHandle::toCommand() const noexcept
    {
        if (!m_handleIndex) return nullptr;

        auto* entry = s_pool.getEntry(m_handleIndex);
        if (!entry) return nullptr;

        if (entry->m_data.m_id && entry->m_data.m_id == m_id) {
            return &entry->m_data;
        }

        // TODO KI invalidated; clear iteslf
        //clear();

        return nullptr;
    }

    CommandHandle CommandHandle::allocate(script::command_id id) noexcept
    {
        if (!id) return NULL_HANDLE;

        //std::lock_guard lock(m_lock);

        auto handleIndex = s_pool.allocate();
        if (!handleIndex) return {};

        auto* entry = s_pool.getEntry(handleIndex);

        entry->m_data.m_id = id;

        //m_IdToIndex.insert({ id, handleIndex });

        return { handleIndex, id };
    }

    //CommandHandle CommandHandle::toHandle(script::command_id id) noexcept
    //{
    //    std::lock_guard lock(m_lock);

    //    const auto& it = m_IdToIndex.find(id);
    //    if (it == m_IdToIndex.end()) return {};
    //    return { it->second, id };
    //}

    //CommandEntry* CommandHandle::toCommand(script::command_id id) noexcept
    //{
    //    std::lock_guard lock(m_lock);

    //    const auto& it = m_IdToIndex.find(id);
    //    if (it == m_IdToIndex.end()) return nullptr;
    //    CommandHandle handle{ it->second, id };
    //    return handle.toCommand();
    //}

    void CommandHandle::clear() noexcept
    {
        //std::lock_guard lock(m_lock);

        s_pool.clear(false);
    }

    script::command_id CommandHandle::nextId()
    {
        return ID_GENERATOR.nextId();
    }
}
