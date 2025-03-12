#include "LuaUtil.h"

#include <functional>

#include "script/UtilAPI.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaUtil::bind(sol::state& lua)
    {
        auto t = lua.create_named_table("util");
        t["sid"] = &UtilAPI::lua_sid;
        t["sid_name"] = &UtilAPI::lua_sid_name;

        t.set_function(
            "lerp",
            sol::overload(
                [](float a, float b, float t) {
                    return std::lerp(a, b, t);
                },
                [](double a, double b, float t) {
                    return std::lerp(a, b, t);
                },
                [](int a, int b, float t) {
                    return std::lerp(a, b, t);
                }
            ));

        t.set_function(
            "clamp",
            sol::overload(
                [](float v, float min, float max) {
                    return std::clamp(v, min, max);
                },
                [](double v, double min, double max) {
                    return std::clamp(v, min, max);
                },
                [](int v, int min, int max) {
                    return std::clamp(v, min, max);
                }
            ));
    }
}
