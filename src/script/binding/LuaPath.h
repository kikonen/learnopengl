#pragma once

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaPath {
        static void bind(sol::state& lua);
    };
}
