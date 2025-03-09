#include "LuaNode.h"

#include "model/Node.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaNode::bind(sol::state& state)
    {
        sol::usertype<Node> t = state.new_usertype<Node>("Node");

        t["get_id"] = &Node::lua_get_id;
        t["get_name"] = &Node::lua_get_name;
        t["get_clone_index"] = &Node::lua_get_clone_index;
        t["get_pos"] = &Node::lua_get_pos;
    }
}
