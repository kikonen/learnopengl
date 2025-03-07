#include "LuaNode.h"

#include "model/Node.h"

namespace script
{
    void LuaNode::bind(sol::state& state)
    {
        state.new_usertype<Node>("Node",
            "get_id", &Node::lua_get_id,
            "get_name", &Node::lua_get_name,
            "get_clone_index", &Node::lua_get_clone_index,
            "get_pos",&Node::lua_get_pos
            );

        //state.new_usertype<Node>("Node");

        //const auto& ut = state["Node"];

        //ut["get_id"] = &Node::lua_getId;
        //ut["get_name"] = &Node::lua_getName;

        //ut["get_clone_index"] = &Node::lua_getCloneIndex;
        //ut["get_pos"] = &Node::lua_getPos;
    }
}
