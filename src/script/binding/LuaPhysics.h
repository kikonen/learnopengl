#pragma once

#include <glm/glm.hpp>

#include "script/lua_binding.h"

namespace script::binding
{
    struct LuaPhysics {
        static void bind(sol::state& lua);
    };
}
