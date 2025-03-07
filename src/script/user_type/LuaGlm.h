#pragma once

#include "script/lua_binding.h"

namespace script
{
    struct LuaGlm {
        void bind(sol::state& state);
    };
}
