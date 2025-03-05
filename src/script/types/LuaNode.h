#pragma once

#include <sol/sol.hpp>

namespace script
{
    struct LuaNode {
        void bind(sol::state& state);
    };
}
