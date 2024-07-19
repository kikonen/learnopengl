#pragma once

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

//#define SF(x) (x == -0.0 ? 0.f : x)
#define SF(x) x

//
// https://fmt.dev/latest/contents.html
// https://fmt.dev/latest/api.html#format-api
//


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
                "({:.3f}, {:.3f}, {:.3f})",
                SF(p.x), SF(p.y), SF(p.z))
            : fmt::format_to(
                ctx.out(),
                "({:.3e}, {:.3e}, {:.3e})",
                SF(p.x), SF(p.y), SF(p.z));
    }
};

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
                "({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                SF(p.x), SF(p.y), SF(p.z), SF(p.w))
            : fmt::format_to(
                ctx.out(),
                "({:.3e}, {:.3e}, {:.3e}, {:.3e})",
                SF(p.x), SF(p.y), SF(p.z), SF(p.w));
    }
};

template <> struct fmt::formatter<glm::uvec4> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::uvec4& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "({}, {}, {}, {})",
            SF(p.x), SF(p.y), SF(p.z), SF(p.w));
    }
};

template <> struct fmt::formatter<glm::uvec3> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::uvec3& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "({}, {}, {})",
            SF(p.x), SF(p.y), SF(p.z));
    }
};

template <> struct fmt::formatter<glm::uvec2> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::uvec2& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "({}, {})",
            SF(p.x), SF(p.y));
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
                SF(m[0].x), SF(m[0].y), SF(m[0].z),
                SF(m[1].x), SF(m[1].y), SF(m[1].z),
                SF(m[2].x), SF(m[2].y), SF(m[2].z),
                SF(m[3].x), SF(m[3].y), SF(m[3].z))
            : fmt::format_to(
                ctx.out(),
                "[({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e})]",
                SF(m[0].x), SF(m[0].y), SF(m[0].z),
                SF(m[1].x), SF(m[1].y), SF(m[1].z),
                SF(m[2].x), SF(m[2].y), SF(m[2].z),
                SF(m[3].x), SF(m[3].y), SF(m[3].z));
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
                SF(m[0].x), SF(m[0].y), SF(m[0].z), SF(m[0].w),
                SF(m[1].x), SF(m[1].y), SF(m[1].z), SF(m[1].w),
                SF(m[2].x), SF(m[2].y), SF(m[2].z), SF(m[2].w),
                SF(m[3].x), SF(m[3].y), SF(m[3].z), SF(m[3].w))
            : fmt::format_to(
                ctx.out(),
                "[({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e}), ({:.1e}, {:.1e}, {:.1e}, {:.1e})]",
                SF(m[0].x), SF(m[0].y), SF(m[0].z), SF(m[0].w),
                SF(m[1].x), SF(m[1].y), SF(m[1].z), SF(m[1].w),
                SF(m[2].x), SF(m[2].y), SF(m[2].z), SF(m[2].w),
                SF(m[3].x), SF(m[3].y), SF(m[3].z), SF(m[3].w));
    }
};

template <> struct fmt::formatter<glm::quat> {
    // Presentation format: 'f' - fixed, 'e' - exponential.
    char presentation = 'f';

    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

        if (it != end && *it != '}') throw fmt::format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(const glm::quat& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return presentation == 'f'
            ? fmt::format_to(
                ctx.out(),
                "({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                SF(p.x), SF(p.y), SF(p.z), SF(p.w))
            : fmt::format_to(
                ctx.out(),
                "({:.3e}, {:.3e}, {:.3e}, {:.3e})",
                SF(p.x), SF(p.y), SF(p.z), SF(p.w));
    }
};
