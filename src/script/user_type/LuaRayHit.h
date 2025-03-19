#pragma once

#include "script/lua_binding.h"

namespace script
{
    struct LuaRayHit {
        static void bind(sol::state& lua);
    };
}
