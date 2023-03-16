#pragma once

#include <variant>

class UpdateContext;
class Node;

namespace event {
    struct NodeAdd {
        Node* m_node{ nullptr };
        void dispatch(const UpdateContext& ctx);
    };

    using EventRef = std::variant<NodeAdd>;
}
