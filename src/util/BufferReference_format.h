#pragma once

#include <fmt/format.h>

#include "BufferReference.h"

template <>
struct fmt::formatter<util::BufferReference> : fmt::formatter<std::string>
{
    auto format(const util::BufferReference& ref, fmt::format_context& ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("[offset={}, size={}]", ref.offset, ref.size), ctx);
    }
};
