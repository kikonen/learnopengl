#pragma once

#include "ki/size.h"

#include "script/lua_binding.h"

namespace pool {
    struct NodeHandle;
}

namespace script::api
{
    class SceneAPI {
    public:
        SceneAPI();
        ~SceneAPI();

        pool::NodeHandle lua_find_node(
            const sol::table& lua_opt) const noexcept;

        std::vector<pool::NodeHandle> lua_find_nodes(
            const sol::table& lua_opt) const noexcept;

        pool::NodeHandle lua_create_node(
            const sol::table& lua_opt);

        pool::NodeHandle lua_clone_node(
            const sol::table& lua_opt);

        bool lua_delete_node(
            pool::NodeHandle nodeId);
    };
}
