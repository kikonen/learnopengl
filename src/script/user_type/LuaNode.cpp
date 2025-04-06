#include "LuaNode.h"

#include "script/NodeAPI.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaNode::bind(sol::state& lua)
    {
        sol::usertype<script::NodeAPI> t = lua.new_usertype<script::NodeAPI>("Node");

        t["get_id"] = &script::NodeAPI::lua_get_id;
        t["get_name"] = &script::NodeAPI::lua_get_name;
        t["get_clone_index"] = &script::NodeAPI::lua_get_clone_index;
        t["get_pos"] = &script::NodeAPI::lua_get_pos;
        t["get_front"] = &script::NodeAPI::lua_get_front;
        t["get_model_matrix"] = &script::NodeAPI::lua_get_model_matrix;

        t.set_function(
            "str",
            [](const script::NodeAPI& v) {
                return v.str();
            }
        );

        t.set_function(
            "__tostring",
            [](const script::NodeAPI& v) {
                return v.str();
            }
        );
    }
}
