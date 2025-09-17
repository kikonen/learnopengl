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
        pool::NodeHandle nodeHandle{};
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
                opt.nodeHandle = script::readHandle(value);
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
        : NodeAPI{ pool::NodeHandle::NULL_HANDLE }
    {
    }

    NodeAPI::NodeAPI(
        pool::NodeHandle handle)
        : m_handle{ handle }
    {
    }

    NodeAPI::~NodeAPI() = default;

    std::string NodeAPI::str() const noexcept
    {
        return m_handle.toNode()->str();
    }

    ki::node_id NodeAPI::lua_get_id() const noexcept
    {
        return m_handle.m_id;
    }

    pool::NodeHandle NodeAPI::lua_find_child(
        const sol::table& lua_opt) const noexcept
    {
        const auto opt = readNodeOptions(lua_opt);

        auto handle = selectHandle(opt.nodeHandle, m_handle, opt.tagId);
        return handle;
    }

    const std::string& NodeAPI::lua_get_type_name() const noexcept
    {
        return m_handle.toNode()->getType()->getName();
    }

    const std::string& NodeAPI::lua_get_name() const noexcept
    {
        return m_handle.toNode()->getName();
    }

    int NodeAPI::lua_get_clone_index() const noexcept
    {
        //return m_cloneIndex;
        return 0;
    }

    const glm::vec3& NodeAPI::lua_get_pos() const noexcept
    {
        return getState().getPosition();
    }

    const glm::vec3& NodeAPI::lua_get_front() const noexcept
    {
        return getState().getViewFront();
    }

    const glm::mat4& NodeAPI::lua_get_model_matrix() const noexcept
    {
        return getState().getModelMatrix();
    }

    const NodeState& NodeAPI::getState() const
    {
        return NodeRegistry::get().getState(m_handle.m_handleIndex);
    }
}
