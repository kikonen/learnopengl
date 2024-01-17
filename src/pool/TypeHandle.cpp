#include "TypeHandle.h"

#include <vector>
#include <mutex>

#include "ki/size.h"

#include "mesh/MeshType.h"

#include "Pool.hpp"
//#include "IdGenerator.h"

namespace {
    //IdGenerator<ki::type_id> ID_GENERATOR;

    pool::Pool<mesh::MeshType> s_pool;
}

namespace pool {
    mesh::MeshType* TypeHandle::toType() const noexcept
    {
        auto& entry = s_pool.getEntry(m_handleIndex);
        if (entry.m_data.m_id && entry.m_data.m_id == m_id) {
            return &entry.m_data;
        }
        return nullptr;
    }

    TypeHandle TypeHandle::allocate(ki::type_id id) noexcept
    {
        auto handleIndex = s_pool.allocate();
        auto& entry = s_pool.getEntry(handleIndex);

        entry.m_data.m_id = id;
        entry.m_data.m_handleIndex = handleIndex;

        return { handleIndex, id };
    }
}
