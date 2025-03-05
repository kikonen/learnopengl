#pragma once

#include <glm/glm.hpp>

#include <sol/sol.hpp>

namespace script
{
    struct LuaUtil {
        void bind(sol::state& state);
    };
}
