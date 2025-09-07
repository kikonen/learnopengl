#include "SceneAPI.h"

#include <glm/glm.hpp>

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"
#include "model/CompositeBuilder.h"

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

    struct CreateOptions {
        ki::type_id typeId;
        ki::node_id nodeId;
        ki::node_id parentId{ 0 };
        ki::socket_id socketId{ 0 };
        ki::tag_id tagId{ 0 };
        glm::vec3 pos{ 0.f };
        glm::vec3 rot{ 0.f };
        glm::vec3 scale{ 1.f };
    };

    struct NodeOptions {
        ki::type_id typeId{ 0 };
        ki::node_id nodeId{ 0 };
        ki::node_id parentId{ 0 };
        ki::socket_id socketId{ 0 };
        ki::tag_id tagId{ 0 };
        glm::vec3 pos{ 0.f };
        glm::vec3 rot{ 0.f };
        glm::vec3 scale{ 1.f };
    };

    CreateOptions readCreateOptions(const sol::table& lua_opt)
    {
        CreateOptions opt;

        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == OPT_TYPE) {
                opt.typeId = script::readSID(value);
            }
            else if (k == OPT_NODE) {
                opt.nodeId = script::readSID(value);
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

    NodeOptions readNodeOptions(const sol::table& lua_opt)
    {
        NodeOptions opt;

        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == OPT_TYPE) {
                opt.typeId = script::readSID(value);
            }
            else if (k == OPT_NODE) {
                opt.nodeId = script::readSID(value);
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
    SceneAPI::SceneAPI()
    {
    }

    SceneAPI::~SceneAPI() = default;

    ki::node_id SceneAPI::lua_find_node(
        const sol::table& lua_opt) const noexcept
    {
        const auto opt = readNodeOptions(lua_opt);

        auto handle = NodeRegistry::get().findTagged(opt.tagId);
        return handle.toId();
    }

    std::vector<uint32_t> SceneAPI::lua_find_nodes(
        const sol::table& lua_opt) const noexcept
    {
        const auto opt = readNodeOptions(lua_opt);

        auto handles = NodeRegistry::get().findTaggedAll(opt.tagId);

        std::vector<uint32_t> nodeIds;
        nodeIds.reserve(handles.size());
        for (auto handle : handles) {
            nodeIds.push_back(handle.toId());
        }
        return nodeIds;
    }

    ki::node_id SceneAPI::lua_create_node(
        const sol::table& lua_opt)
    {
        const CreateOptions& opt = readCreateOptions(lua_opt);

        const auto typeId = opt.typeId;
        const auto* type = pool::TypeHandle::toType(typeId);
        if (!type) return 0;

        ki::node_id nodeId;
        {
            CompositeBuilder builder{ NodeRegistry::get() };
            CreateState state{
                opt.pos,
                opt.scale,
                util::degreesToQuat(opt.rot),
                opt.tagId};

            nodeId = builder.build(opt.parentId, 0, type, state);
            if (nodeId) {
                builder.attach();
            }
        }

        return nodeId;
    }

    ki::node_id SceneAPI::lua_clone_node(
        const sol::table& lua_opt)
    {
        const CreateOptions& opt = readCreateOptions(lua_opt);

        const auto nodeId = opt.nodeId;
        const auto* node = pool::NodeHandle::toNode(nodeId);
        if (!node) return 0;

        const auto& state = node->getState();
        const auto* type = node->getType();

        {
            ki::node_id parentId{ node->getParentHandle().toId() };
            glm::vec3 pos{ state.getPosition() };
            glm::quat quat{ state.getRotation() };
            glm::vec3 scale{ state.getScale() };
            ki::tag_id tagId{ state.m_tagId };

            CreateState state{
                pos,
                scale,
                quat,
                tagId};

            CompositeBuilder builder{ NodeRegistry::get() };
            if (builder.build(parentId, 0, type, state)) {
                auto rootHandle = builder.attach();
            }
        }

        return nodeId;
    }

    bool SceneAPI::lua_delete_node(
        ki::node_id nodeId)
    {
        auto handle = pool::NodeHandle::toHandle(nodeId);
        if (!handle) return false;

        auto& nodeRegistry = NodeRegistry::get();
        nodeRegistry.detachNode(handle);

        return true;
    }
}
