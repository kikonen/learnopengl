#pragma once

#include <glm/glm.hpp>

#include <sol/sol.hpp>

namespace script
{
    struct LuaVec3 {
        void bind(sol::state& state);
    };
}
