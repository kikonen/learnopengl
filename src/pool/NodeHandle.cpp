#include "NodeHandle.h"

#include <vector>
#include <mutex>

#include "ki/size.h"

#include "model/Node.h"

#include "Pool.hpp"
//#include "IdGenerator.h"

namespace {
    //IdGenerator<ki::node_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 100000 };

    pool::Pool<Node> s_pool{ MAX_POOL_SIZE };
}

namespace pool {
    Node* NodeHandle::toNode() const noexcept
    {
        auto& entry = s_pool.getEntry(m_handleIndex);
        if (entry.m_data.m_id && entry.m_data.m_id == m_id) {
            return &entry.m_data;
        }
        return nullptr;
    }

    NodeHandle NodeHandle::allocate(ki::node_id id) noexcept
    {
        auto handleIndex = s_pool.allocate();
        auto& entry = s_pool.getEntry(handleIndex);

        entry.m_data.m_id = id;
        entry.m_data.m_handleIndex = handleIndex;

        return { handleIndex, id };
    }
}
