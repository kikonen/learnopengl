#include "LuaScene.h"

#include "script/api/SceneAPI.h"

namespace script::binding
{
    using namespace script::api;

    // https://thephd.dev/sol3-feature-complete
    void LuaScene::bind(sol::state& lua)
    {
        sol::usertype<SceneAPI> t = lua.new_usertype<SceneAPI>("Scene");

        t["create_node"] = &SceneAPI::lua_create_node;
        t["delete_node"] = &SceneAPI::lua_delete_node;
    }
}
