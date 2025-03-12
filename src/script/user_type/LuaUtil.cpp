#include "LuaUtil.h"

#include "script/UtilAPI.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaUtil::bind(sol::state& lua)
    {
        auto t = lua.create_named_table("util");
        t["sid"] = &UtilAPI::lua_sid;
        t["sid_name"] = &UtilAPI::lua_sid_name;
    }
}
