#pragma once

#include <string>
#include <unordered_map>

namespace loader {
    struct FlagContainer {
        std::unordered_map<std::string, bool> flags{};

        void set(const std::string& flag, bool value)
        {
            flags[flag] = value;
        }

        const bool* getOptional(const std::string& flag) const noexcept
        {
            const auto& it = flags.find(flag);
            return it != flags.end() ? &(it->second)  : nullptr;
        }

        bool getFlag(
            const std::string& flag,
            bool defaultValue) const noexcept
        {
            const auto& e = flags.find(flag);
            if (e != flags.end()) {
                return e->second;
            }
            return defaultValue;
        }
    };
}
