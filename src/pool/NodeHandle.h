#pragma once

#include "ki/size.h"

class Node;

namespace pool {
    class NodeHandle final
    {
        friend class Node;

    public:
        NodeHandle(
            uint32_t handleIndex,
            ki::node_id id
        ) : m_handleIndex(handleIndex),
            m_id(id)
        {}

        Node* toNode() const noexcept;

        static NodeHandle allocate(ki::node_id id) noexcept;

    private:
        const uint32_t m_handleIndex;
        const ki::node_id m_id;
    };
}
