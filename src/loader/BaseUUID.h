#pragma once

#include <vector>
#include <string>

#include <fmt/format.h>

namespace loader {
    using BaseUUID = std::vector<std::string>;
}

template <> struct fmt::formatter<loader::BaseUUID> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const loader::BaseUUID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
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
