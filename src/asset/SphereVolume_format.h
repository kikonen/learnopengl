#pragma once

#include <fmt/format.h>

#include "SphereVolume.h"

//#define SF(x) (x == -0.0 ? 0.f : x)
#define SF(x) x

template <> struct fmt::formatter<SphereVolume>
{
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const SphereVolume& p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(
            ctx.out(),
            "({:.3f}, {:.3f}), {:.3f}), {:.3f})",
            SF(p.x), SF(p.y), SF(p.z), SF(p.radius));
    }   
};
