#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "ki/size.h"


namespace loader {
    struct BaseId {
        std::string m_path;

        bool empty() const {
            return m_path.empty();
        }
    };
}

template <> struct fmt::formatter<loader::BaseId> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const loader::BaseId& id, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto& p = id.m_path;

        if (p.empty()) {
            return ctx.out();
        }
        else if (p.size() == 1) {
            return fmt::format_to(
                ctx.out(),
                "{}", p[0]);
        }
        else {
            return fmt::format_to(
                ctx.out(),
                "{}-{}", p[0], p[1]);
        }
    }
};
