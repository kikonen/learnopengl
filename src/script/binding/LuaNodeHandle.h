#pragma once

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaNodeHandle {
        static void bind(sol::state& lua);
    };
}
