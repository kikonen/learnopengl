#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "DocNode.h"

namespace loader {
    struct Document {
        Document(std::unique_ptr<DocNode>&& root)
            : m_root{ std::move(root) }
        {}

        std::unique_ptr<DocNode> m_root;
    };
}
