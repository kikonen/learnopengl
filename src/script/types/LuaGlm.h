#pragma once

#include <sol/sol.hpp>

namespace script
{
    struct LuaGlm {
        void bind(sol::state& state);
    };
}
