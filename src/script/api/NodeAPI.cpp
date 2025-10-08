#include "NodeAPI.h"

#include "ki/size.h"

#include "model/Node.h"
#include "model/NodeState.h"
#include "model/NodeType.h"

#include "registry/NodeRegistry.h"

#include "script/lua_util.h"

namespace {
    const inline std::string OPT_TYPE{ "type" };
    const inline std::string OPT_NODE{ "node" };
    const inline std::string OPT_PARENT{ "parent" };
    const inline std::string OPT_SOCKET{ "socket" };
    const inline std::string OPT_TAG{ "tag" };
    const inline std::string OPT_POS{ "pos" };
    const inline std::string OPT_ROT{ "rot" };
    const inline std::string OPT_SCALE{ "scale" };

    struct NodeOptions {
        ki::type_id typeId{ 0 };
        //pool::NodeHandle nodeHandle{};
        ki::node_id parentId{ 0 };
        ki::socket_id socketId{ 0 };
        ki::tag_id tagId{ 0 };
        glm::vec3 pos{ 0.f };
        glm::vec3 rot{ 0.f };
        glm::vec3 scale{ 1.f };
    };

    NodeOptions readNodeOptions(const sol::table& lua_opt)
    {
        NodeOptions opt;

        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == OPT_TYPE) {
                opt.typeId = script::readSID(value);
            }
            else if (k == OPT_NODE) {
                //opt.nodeHandle = script::readHandle(value);
                throw "node is unsupported";
            }
            else if (k == OPT_PARENT) {
                opt.parentId = script::readSID(value);
            }
            else if (k == OPT_SOCKET) {
                opt.socketId = script::readSID(value);
            }
            else if (k == OPT_TAG) {
                opt.tagId = script::readSID(value);
            }
            else if (k == OPT_POS) {
                opt.pos = value.as<glm::vec3>();
            }
            else if (k == OPT_ROT) {
                opt.rot = value.as<glm::vec3>();
            }
            else if (k == OPT_SCALE) {
                opt.scale = value.as<glm::vec3>();
            }
        });

        return opt;
    }
}

namespace script::api
{
    NodeAPI::NodeAPI()
    {
    }

    NodeAPI::~NodeAPI() = default;

    std::string NodeAPI::str(pool::NodeHandle handle) const noexcept
    {
        return handle.toNode()->str();
    }

    pool::NodeHandle NodeAPI::lua_find_child(
        pool::NodeHandle handle,
        const sol::table& lua_opt) const noexcept
    {
        const auto opt = readNodeOptions(lua_opt);

        handle = selectHandle(handle, opt.tagId);
        return handle;
    }

    const std::string& NodeAPI::lua_get_type_name(
        pool::NodeHandle handle) const noexcept
    {
        return handle.toNode()->getType()->getName();
    }

    const std::string& NodeAPI::lua_get_name(
        pool::NodeHandle handle) const noexcept
    {
        return handle.toNode()->getName();
    }

    int NodeAPI::lua_get_clone_index(pool::NodeHandle handle) const noexcept
    {
        //return m_cloneIndex;
        return 0;
    }

    const glm::vec3& NodeAPI::lua_get_pos(pool::NodeHandle handle) const noexcept
    {
        return getState(handle).getPosition();
    }

    const glm::vec3& NodeAPI::lua_get_front(pool::NodeHandle handle) const noexcept
    {
        return getState(handle).getViewFront();
    }

    const glm::mat4& NodeAPI::lua_get_model_matrix(
        pool::NodeHandle handle) const noexcept
    {
        return getState(handle).getModelMatrix();
    }

    const model::NodeState& NodeAPI::getState(pool::NodeHandle handle) const
    {
        const auto& nodeRegistry = NodeRegistry::get();
        return nodeRegistry.getState(handle.m_handleIndex);
    }

    // https://thephd.dev/sol3-feature-complete
    void NodeAPI::bind(sol::state& lua)
    {
        sol::usertype<NodeAPI> t = lua.new_usertype<NodeAPI>("Node");

        //t["get_id"] = &NodeAPI::lua_get_id;
        t["get_type_name"] = &NodeAPI::lua_get_type_name;
        t["get_name"] = &NodeAPI::lua_get_name;
        t["get_clone_index"] = &NodeAPI::lua_get_clone_index;
        t["get_pos"] = &NodeAPI::lua_get_pos;
        t["get_front"] = &NodeAPI::lua_get_front;
        t["get_model_matrix"] = &NodeAPI::lua_get_model_matrix;
        t["find_child"] = &NodeAPI::lua_find_child;

        t.set_function(
            "str",
            [](const NodeAPI& v, pool::NodeHandle handle) {
                return v.str(handle);
            }
        );

        t.set_function(
            "__tostring",
            [](const NodeAPI& v) {
                return "<NodeAPI>";
            }
        );
    }
}
