#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "Node.h"

namespace loader {
    struct Document {
        Document(std::unique_ptr<Node>&& root)
            : m_root{ std::move(root) }
        {}

        std::unique_ptr<Node> m_root;
    };
}
