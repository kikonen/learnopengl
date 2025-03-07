#pragma once

#include <glm/glm.hpp>

#include "script/lua_binding.h"

namespace script
{
    struct LuaUtil {
        void bind(sol::state& state);
    };
}
