#include "LuaUtil.h"

#include "script/UtilAPI.h"

namespace script
{
    void LuaUtil::bind(sol::state& state)
    {
        state["util"] = state.create_table_with(
            "sid", &UtilAPI::lua_sid,
            "sid_name", &UtilAPI::lua_sid_name
        );
    }
}
