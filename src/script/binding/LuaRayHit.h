#pragma once

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaRayHit {
        static void bind(sol::state& lua);
    };
}
