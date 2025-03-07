#pragma once

#include <glm/glm.hpp>

#include "script/lua_binding.h"

namespace script
{
    struct LuaVec3 {
        void bind(sol::state& state);
    };
}
