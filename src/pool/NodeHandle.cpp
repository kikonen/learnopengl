#include "NodeHandle.h"

#include <vector>
#include <mutex>
#include <unordered_map>

#include "ki/size.h"

#include "model/Node.h"

#include "Pool_impl.h"
//#include "IdGenerator.h"

namespace {
    //IdGenerator<ki::node_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 100000 };

    std::mutex m_lock;
    pool::Pool<Node> s_pool{ MAX_POOL_SIZE };

    std::unordered_map<ki::node_id, uint32_t> m_IdToIndex;
}

namespace pool {
    NodeHandle NodeHandle::NULL_HANDLE{ 0, 0 };

    NodeHandle& NodeHandle::operator=(const Node* node) noexcept
    {
        if (node) {
            m_handleIndex = node->m_handle.m_handleIndex;
            m_id = node->m_handle.m_id;
        }
        return *this;
    }

    Node* NodeHandle::toNode() const noexcept
    {
        if (!m_handleIndex) return nullptr;

        auto* entry = s_pool.getEntry(m_handleIndex);
        if (!entry) return nullptr;

        if (entry->m_data.m_handle.m_id && entry->m_data.m_handle.m_id == m_id) {
            return &entry->m_data;
        }

        // TODO KI invalidated; clear itself
        //clear();

        return nullptr;
    }

    NodeHandle NodeHandle::allocate(ki::node_id id) noexcept
    {
        if (!id) return NULL_HANDLE;

        std::lock_guard lock(m_lock);

        auto handleIndex = s_pool.allocate();
        if (!handleIndex) return {};

        auto* entry = s_pool.getEntry(handleIndex);

        NodeHandle handle{ handleIndex, id };
        entry->m_data.m_handle = handle;

        m_IdToIndex.insert({ id, handleIndex });

        return handle;
    }

    NodeHandle NodeHandle::toHandle(ki::node_id id) noexcept
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return {};
        return { it->second, id };
    }

    Node* NodeHandle::toNode(ki::node_id id) noexcept
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return nullptr;
        NodeHandle handle{ it->second, id };
        return handle.toNode();
    }

    void NodeHandle::clear() noexcept
    {
        std::lock_guard lock(m_lock);

        s_pool.clear(false);
    }
}
