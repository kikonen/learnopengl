#pragma once

#include <sol/sol.hpp>

namespace script
{
    struct LuaGlm {
        static void bind(sol::state& lua);
    };
}
