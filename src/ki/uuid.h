#pragma once

//#include <fmt/format.h>

// https://github.com/mariusbancila/stduuid
#define UUID_SYSTEM_GENERATOR 1
#include <stduuid/uuid.h>

#define KI_UUID(x) (uuids::uuid::from_string(x).value())
#define KI_UUID_STR(x) (x.is_nil() ? "NULL" : uuids::to_string(x))

//// @see https://fmt.dev/latest/api.html#udt
//template <> struct fmt::formatter<uuids::uuid> {
//    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
//    }
//
//    template <typename FormatContext>
//    auto format(const uuids::uuid& a, FormatContext& ctx) const -> decltype(ctx.out()) {
//        return uuids::to_string(a);
//    }
//};
//
