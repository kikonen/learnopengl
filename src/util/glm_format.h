#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>


//
// https://fmt.dev/latest/contents.html
// https://fmt.dev/latest/api.html#format-api
// 

template <> struct fmt::formatter<glm::vec4> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::vec4& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return presentation == 'f'
            ? fmt::format_to(
                ctx.out(),
                "({:.1f}, {:.1f}, {:.1f}, {:.1f})",
                p.x, p.y, p.z, p.w)
            : fmt::format_to(
                ctx.out(),
                "({:.1e}, {:.1e}, {:.1e}, {:.1e})",
                p.x, p.y, p.z, p.w);
    }
};

template <> struct fmt::formatter<glm::vec3> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::vec3& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return presentation == 'f'
            ? fmt::format_to(
                ctx.out(),
                "({:.1f}, {:.1f}, {:.1f})",
                p.x, p.y, p.z)
            : fmt::format_to(
                ctx.out(),
                "({:.1e}, {:.1e}, {:.1e})",
                p.x, p.y, p.z);
    }
};

template <> struct fmt::formatter<glm::mat3> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::mat3& m, FormatContext& ctx) const -> decltype(ctx.out()) {
        return presentation == 'f'
            ? fmt::format_to(
                ctx.out(),
                "[({:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f})]",
                m[0].x, m[0].y, m[0].z,
                m[1].x, m[1].y, m[1].z,
                m[2].x, m[2].y, m[2].z,
                m[3].x, m[3].y, m[3].z)
            : fmt::format_to(
                ctx.out(),
                "[({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e})]",
                m[0].x, m[0].y, m[0].z,
                m[1].x, m[1].y, m[1].z,
                m[2].x, m[2].y, m[2].z,
                m[3].x, m[3].y, m[3].z);
    }
};

template <> struct fmt::formatter<glm::mat4> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::mat4& m, FormatContext& ctx) const -> decltype(ctx.out()) {
        return presentation == 'f'
            ? fmt::format_to(
                ctx.out(),
                "[({:.1f}, {:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}, {:.1f}), ({:.1f}, {:.1f}, {:.1f}, {:.1f})]",
                m[0].x, m[0].y, m[0].z, m[0].w,
                m[1].x, m[1].y, m[1].z, m[1].w,
                m[2].x, m[2].y, m[2].z, m[2].w,
                m[3].x, m[3].y, m[3].z, m[3].w)
            : fmt::format_to(
                ctx.out(),
                "[({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e})]",
                m[0].x, m[0].y, m[0].z, m[0].w,
                m[1].x, m[1].y, m[1].z, m[1].w,
                m[2].x, m[2].y, m[2].z, m[2].w,
                m[3].x, m[3].y, m[3].z, m[3].w);
    }
};
