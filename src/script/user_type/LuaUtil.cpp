#include "LuaUtil.h"

#include "script/UtilAPI.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaUtil::bind(sol::state& state)
    {
        state["util"] = state.create_table_with(
            "sid", &UtilAPI::lua_sid,
            "sid_name", &UtilAPI::lua_sid_name
        );
    }
}
