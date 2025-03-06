#include "LuaGlm.h"

#include <glm/glm.hpp>

#include "script/UtilAPI.h"

namespace {
    auto lua_normalize(const glm::vec3& v) {
        return std::make_shared<glm::vec3>(glm::normalize(v));
    }
}

namespace script
{
    void LuaGlm::bind(sol::state& state)
    {
        state["glm"] = state.create_table_with(
            "normalize", &lua_normalize
        );
    }
}
