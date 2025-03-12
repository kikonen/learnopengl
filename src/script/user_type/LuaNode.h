#pragma once

#include "script/lua_binding.h"

namespace script
{
    struct LuaNode {
        static void bind(sol::state& lua);
    };
}
