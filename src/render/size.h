#pragma once

namespace render {
    inline constexpr unsigned int KIND_NONE{ 0 };
    inline constexpr unsigned int KIND_SOLID{ 1 << 0 };
    inline constexpr unsigned int KIND_ALPHA{ 1 << 1 };
    inline constexpr unsigned int KIND_BLEND{ 1 << 2 };
    inline constexpr unsigned int KIND_ALL{ KIND_SOLID | KIND_ALPHA | KIND_BLEND };

    // Render route: which pass family a drawable belongs to. Drawable buckets are partitioned
    // by route so a route-exclusive pass (forward, g-buffer, OIT) sweeps only its own set.
    inline constexpr unsigned int ROUTE_DEFERRED{ 1 << 0 };   // g-buffer, pre-depth, OIT(blend)
    inline constexpr unsigned int ROUTE_FORWARD{ 1 << 1 };    // forward pass, effect(blend)
    inline constexpr unsigned int ROUTE_ALL{ ROUTE_DEFERRED | ROUTE_FORWARD };
}
