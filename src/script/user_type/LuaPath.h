#pragma once

#include "script/lua_binding.h"

namespace script
{
    struct LuaPath {
        static void bind(sol::state& lua);
    };
}
