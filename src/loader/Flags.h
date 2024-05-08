#pragma once

#include <string>
#include <map>

namespace loader
{
    struct Flags {
        std::map<std::string, bool> m_flags;

        void mark(const std::string& f) {
            set(f, true);
        }

        void set(const std::string& f, bool value) {
            m_flags.insert({ f, value });
        }

        bool exist(const std::string& f) const noexcept {
            return m_flags.find(f) != m_flags.end();
        }

        bool get(const std::string& f) const noexcept {
            const auto& it = m_flags.find(f);
            return it != m_flags.end() ? it->second : false;
        }
    };
}
