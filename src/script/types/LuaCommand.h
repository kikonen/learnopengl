#pragma once

#include <glm/glm.hpp>

#include <sol/sol.hpp>

namespace script
{
    struct LuaCommand {
        void bind(sol::state& state);
    };
}
