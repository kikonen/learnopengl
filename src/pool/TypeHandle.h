#pragma once

#include "ki/size.h"

namespace mesh {
    class MeshType;
}

namespace pool {
    class TypeHandle final
    {
        friend class mesh::MeshType;

    public:
        TypeHandle(
            uint32_t handleIndex,
            ki::type_id id
        ) : m_handleIndex(handleIndex),
            m_id(id)
        {}

        mesh::MeshType* toType() const noexcept;

        static TypeHandle allocate(ki::type_id id) noexcept;

    private:
        const uint32_t m_handleIndex;
        const ki::type_id m_id;
    };
}
