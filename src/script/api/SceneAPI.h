#pragma once

#include "ki/size.h"

#include "script/lua_binding.h"

namespace script::api
{
    class SceneAPI {
    public:
        SceneAPI();
        ~SceneAPI();

        ki::node_id lua_create_node(
            const sol::table& lua_opt);

        ki::node_id lua_clone_node(
            const sol::table& lua_opt);

        bool lua_delete_node(
            ki::node_id nodeId);
    };
}
