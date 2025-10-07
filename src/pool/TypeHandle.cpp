#include "TypeHandle.h"

#include <vector>
#include <mutex>
#include <unordered_map>

#include "ki/size.h"

#include "model/NodeType.h"

#include "Pool_impl.h"
#include "IdGenerator.h"

namespace {
    //IdGenerator<ki::type_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 100000 };

    std::mutex m_lock;
    pool::Pool<model::NodeType> s_pool{ MAX_POOL_SIZE };

    std::unordered_map<ki::type_id, uint32_t> s_IdToIndex;
}

namespace pool {
    TypeHandle TypeHandle::NULL_HANDLE{ 0, 0 };

    TypeHandle& TypeHandle::operator=(const model::NodeType* type) noexcept
    {
        if (type) {
            m_handleIndex = type->m_handle.m_handleIndex;
            m_id = type->m_handle.m_id;
        }
        return *this;
    }

    model::NodeType* TypeHandle::toType() const noexcept
    {
        if (!m_handleIndex) return nullptr;

        auto* entry = s_pool.getEntry(m_handleIndex);
        if (!entry) return nullptr;

        if (entry->m_data.m_handle.m_id && entry->m_data.m_handle.m_id == m_id) {
            return &entry->m_data;
        }

        // TODO KI invalidated; clear iteslf
        //clear();

        return nullptr;
    }

    TypeHandle TypeHandle::allocate(ki::type_id id) noexcept
    {
        //auto id = ID_GENERATOR.nextId();

        std::lock_guard lock(m_lock);

        auto handleIndex = s_pool.allocate();
        if (!handleIndex) return {};

        auto* entry = s_pool.getEntry(handleIndex);

        TypeHandle handle{ handleIndex, id };
        entry->m_data.m_handle = handle;

        s_IdToIndex.insert({ id, handleIndex });

        return { handleIndex, id };
    }

    TypeHandle TypeHandle::toHandle(ki::type_id id) noexcept
    {
        std::lock_guard lock(m_lock);

        const auto& it = s_IdToIndex.find(id);
        if (it == s_IdToIndex.end()) return {};
        return { it->second, id };
    }

    model::NodeType* TypeHandle::toType(ki::type_id id) noexcept
    {
        std::lock_guard lock(m_lock);

        const auto& it = s_IdToIndex.find(id);
        if (it == s_IdToIndex.end()) return nullptr;
        TypeHandle handle{ it->second, id };
        return handle.toType();
    }

    void TypeHandle::clear() noexcept
    {
        std::lock_guard lock(m_lock);

        s_IdToIndex.clear();
        s_pool.clear(false);
    }
}
