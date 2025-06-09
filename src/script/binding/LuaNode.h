#pragma once

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaNode {
        static void bind(sol::state& lua);
    };
}
