#include "TypeHandle.h"

#include <vector>
#include <mutex>
#include <map>

#include "ki/size.h"

#include "mesh/MeshType.h"

#include "Pool.hpp"
#include "IdGenerator.h"

namespace {
    IdGenerator<ki::type_id> ID_GENERATOR;

    constexpr size_t MAX_POOL_SIZE{ 100000 };

    pool::Pool<mesh::MeshType> s_pool{ MAX_POOL_SIZE };

    std::map<ki::type_id, uint32_t> m_IdToIndex;
}

namespace pool {
    TypeHandle TypeHandle::NULL_HANDLE{ 0, 0 };

    TypeHandle& TypeHandle::operator=(const mesh::MeshType* type) noexcept
    {
        if (type) {
            m_handleIndex = type->m_handleIndex;
            m_id = type->m_id;
        }
        return *this;
    }

    mesh::MeshType* TypeHandle::toType() const noexcept
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

    TypeHandle TypeHandle::allocate() noexcept
    {
        auto id = ID_GENERATOR.nextId();

        auto handleIndex = s_pool.allocate();
        auto& entry = s_pool.getEntry(handleIndex);

        entry.m_data.m_id = id;
        entry.m_data.m_handleIndex = handleIndex;

        m_IdToIndex.insert({ id, handleIndex });

        return { handleIndex, id };
    }

    TypeHandle TypeHandle::toHandle(ki::type_id id) noexcept
    {
        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return {};
        return { it->second, id };
    }

    mesh::MeshType* TypeHandle::toType(ki::type_id id) noexcept
    {
        const auto& it = m_IdToIndex.find(id);
        if (it == m_IdToIndex.end()) return nullptr;
        TypeHandle handle{ it->second, id };
        return handle.toType();
    }

    void TypeHandle::clear() noexcept
    {
        s_pool.clear();
    }
}
