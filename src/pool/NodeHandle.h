#pragma once

#include <string>
#include <vector>

#include "ki/size.h"

class Node;

namespace pool {
    struct NodeHandle final
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

        // NOTE KI more useful to default to ki::node_id
        operator int() const {
            return m_id;
        }

        inline std::string str() const noexcept
        {
            return "[" + std::to_string(m_id) + "." + std::to_string(m_handleIndex) + "]";
        }

        void reset() noexcept {
            m_handleIndex = 0;
            m_id = 0;
        }

        void release() const;

        Node* toNode() const noexcept;
        ki::node_id toId() const noexcept { return m_id; }
        int toIndex() const noexcept { return m_handleIndex; }

        // @return true if found
        bool removeFrom(
            std::vector<pool::NodeHandle>& handles) const;

        void addTo(
            std::vector<pool::NodeHandle>& handles) const;

        static NodeHandle allocate(ki::node_id id) noexcept;

        static NodeHandle toHandle(ki::node_id id) noexcept;

        static Node* toNode(ki::node_id id) noexcept;

        static void clear() noexcept;

    public:
        static NodeHandle NULL_HANDLE;

    //private:
        uint32_t m_handleIndex;
        ki::node_id m_id;
    };
}

template <>
struct std::hash<pool::NodeHandle>
{
    size_t operator()(const pool::NodeHandle& k) const
    {
        // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        return ((std::hash<int>()(k.m_handleIndex)
            ^ (std::hash<int>()(k.m_id) << 1)) >> 1);
    }
};
