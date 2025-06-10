#pragma once

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaScene {
        static void bind(sol::state& lua);
    };
}
