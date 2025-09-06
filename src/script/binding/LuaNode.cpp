#include "LuaNode.h"

#include "script/api/NodeAPI.h"

namespace script::binding
{
    using namespace script::api;

    // https://thephd.dev/sol3-feature-complete
    void LuaNode::bind(sol::state& lua)
    {
        sol::usertype<NodeAPI> t = lua.new_usertype<NodeAPI>("Node");

        t["get_id"] = &NodeAPI::lua_get_id;
        t["get_type_name"] = &NodeAPI::lua_get_type_name;
        t["get_name"] = &NodeAPI::lua_get_name;
        t["get_clone_index"] = &NodeAPI::lua_get_clone_index;
        t["get_pos"] = &NodeAPI::lua_get_pos;
        t["get_front"] = &NodeAPI::lua_get_front;
        t["get_model_matrix"] = &NodeAPI::lua_get_model_matrix;
        t["find_child"] = &NodeAPI::lua_find_child;

        t.set_function(
            "str",
            [](const NodeAPI& v) {
                return v.str();
            }
        );

        t.set_function(
            "__tostring",
            [](const NodeAPI& v) {
                return v.str();
            }
        );
    }
}
