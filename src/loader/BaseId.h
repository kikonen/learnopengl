#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "ki/size.h"


namespace loader {
    struct BaseId {
        std::string m_path;

        const std::string& str() const noexcept
        {
            return m_path;
        }

        bool empty() const {
            return m_path.empty();
        }

        bool operator==(const BaseId& o) const noexcept
        {
            return m_path == o.m_path;
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
        const auto& p = id.m_path;

        if (p.empty()) {
            return ctx.out();
        }
        else {
            return fmt::format_to(
                ctx.out(),
                "{}", p);
        }
    }
};
