#pragma once

#include "ki/size.h"

class Node;

namespace pool {
    class NodeHandle final
    {
        friend class Node;

    public:
        NodeHandle()
            : m_handleIndex{ 0 },
            m_id{ 0 }
        {}

        NodeHandle(
            uint32_t handleIndex,
            ki::node_id id
        ) : m_handleIndex{ handleIndex },
            m_id{ id }
        {}

        NodeHandle(const NodeHandle& o)
            : m_handleIndex{ o.m_handleIndex },
            m_id{ o.m_id }
        {}

        NodeHandle(NodeHandle&& o) noexcept
            : m_handleIndex { o.m_handleIndex },
            m_id { o.m_id }
        {}

        ~NodeHandle() = default;

        NodeHandle& operator=(const NodeHandle& o)
        {
            if (&o == this) return *this;
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        NodeHandle& operator=(NodeHandle&& o) noexcept
        {
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        NodeHandle& operator=(const Node* node) noexcept;

        bool operator==(const NodeHandle& o) const noexcept
        {
            return m_handleIndex == o.m_handleIndex &&
                m_id == o.m_id;
        }

        bool present() const noexcept { return m_handleIndex > 0; }
        bool isNull() const noexcept { return m_handleIndex == 0;  }
        operator int() const { return m_handleIndex; }

        void reset() noexcept {
            m_handleIndex = 0;
            m_id = 0;
        }

        Node* toNode() const noexcept;
        ki::node_id toId() const noexcept { return m_id; }

        static NodeHandle allocate(ki::node_id id) noexcept;

        static NodeHandle toHandle(ki::node_id id) noexcept;

        static Node* toNode(ki::node_id id) noexcept;

        static void clear() noexcept;

    public:
        static NodeHandle NULL_HANDLE;

    private:
        uint32_t m_handleIndex;
        ki::node_id m_id;
    };
}
