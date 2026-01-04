#pragma once

namespace render
{
    // @see backendr/DrawOptions.h
    inline constexpr unsigned int INSTANCE_FLAG_NONE{ 0 };
    inline constexpr unsigned int INSTANCE_BILLBOARD_BIT{ 1 << 0 };
    inline constexpr unsigned int INSTANCE_TESSELATION_BIT{ 1 << 1 };
    inline constexpr unsigned int INSTANCE_PRE_DEPTH_BIT{ 1 << 2 };
}
