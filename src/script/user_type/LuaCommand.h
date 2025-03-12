#pragma once

#include <glm/glm.hpp>

#include "script/lua_binding.h"

namespace script
{
    struct LuaCommand {
        static void bind(sol::state& lua);
    };
}
