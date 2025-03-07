#pragma once

#include "script/lua_binding.h"

namespace script
{
    struct LuaNode {
        void bind(sol::state& state);
    };
}
