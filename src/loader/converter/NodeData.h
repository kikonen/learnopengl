#pragma once

#include <vector>
#include <string>
#include <memory>

#include "base.h"

namespace loader {
    struct NodeData {
        const loader::DocNode& findNode(const std::string& key) const noexcept;

        //template<typename T>
        //auto as() const noexcept;

        const std::string& asString() const noexcept {
            return m_value;
        }

        int asInt() const noexcept {
            return std::stoi(m_value);
        }

        float asFloat() const noexcept {
            return std::stof(m_value);
        }

        bool asBool() const noexcept;

        std::string m_value;
        std::vector<loader::DocNode> m_nodes;
    };
}
