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
        TypeHandle()
            : m_handleIndex{ 0 },
            m_id{ 0 }
        {}

        TypeHandle(
            uint32_t handleIndex,
            ki::type_id id
        ) : m_handleIndex{ handleIndex },
            m_id{ id }
        {}

        TypeHandle(const TypeHandle& o)
            : m_handleIndex{ o.m_handleIndex },
            m_id{ o.m_id }
        {}

        TypeHandle(TypeHandle&& o) noexcept
            : m_handleIndex{ o.m_handleIndex },
            m_id{ o.m_id }
        {}

        ~TypeHandle() = default;

        TypeHandle& operator=(const TypeHandle& o)
        {
            if (&o == this) return *this;
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        TypeHandle& operator=(TypeHandle&& o) noexcept
        {
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        TypeHandle& operator=(const mesh::MeshType* type) noexcept;

        bool operator==(const TypeHandle& o) const noexcept
        {
            return m_handleIndex == o.m_handleIndex &&
                m_id == o.m_id;
        }

        bool present() const noexcept { return m_handleIndex > 0; }
        bool isNull() const noexcept { return m_handleIndex == 0; }
        operator int() const { return m_handleIndex; }

        void reset() noexcept {
            m_handleIndex = 0;
            m_id = 0;
        }

        mesh::MeshType* toType() const noexcept;
        ki::type_id toId() const noexcept { return m_id; }

        static TypeHandle allocate() noexcept;

        static TypeHandle toHandle(ki::type_id id) noexcept;

        static mesh::MeshType* toType(ki::type_id id) noexcept;

        static void clear() noexcept;

    public:
        static TypeHandle NULL_HANDLE;

    private:
        uint32_t m_handleIndex;
        ki::type_id m_id;
    };
}
