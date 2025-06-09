#pragma once

#include <sol/sol.hpp>

namespace script::binding
{
    struct LuaGlm {
        static void bind(sol::state& lua);
    };
}
