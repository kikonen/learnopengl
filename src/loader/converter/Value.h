#pragma once

#include <string>

namespace loader {
    struct Value {
        Value(const std::string& data)
            : m_data{ data }
        {}

        const std::string m_data;

        std::string asString() const noexcept {
            return m_data;
        }

        int asInt() const noexcept {
            return std::stoi(m_data);
        }

        float asFloat() const noexcept {
            return std::stof(m_data);
        }
    };
}
