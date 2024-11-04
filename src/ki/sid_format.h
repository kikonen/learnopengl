#pragma once

#include <fmt/format.h>

#include "sid.h"

template <> struct fmt::formatter<ki::StringID> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const ki::StringID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[SID:{}/{}]", p.m_sid, p.getName());
    }
};
