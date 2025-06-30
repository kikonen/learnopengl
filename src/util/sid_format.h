#pragma once

#include <fmt/format.h>

#include "ki/sid.h"

//
// https://fmt.dev/latest/contents.html
// https://fmt.dev/latest/api.html#format-api
//

////////////////////////////////
// DVEC
////////////////////////////////
template <> struct fmt::formatter<ki::StringID> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        return it;
    }

    template <typename FormatContext>
    auto format(const ki::StringID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "<{},{}>",
            p.m_sid, p.getName());
    }
};
