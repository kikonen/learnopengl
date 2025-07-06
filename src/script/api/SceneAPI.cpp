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

namespace {
    const inline std::string OPT_TYPE{ "type" };
    const inline std::string OPT_NODE{ "node" };
    const inline std::string OPT_PARENT{ "parent" };
    const inline std::string OPT_POS{ "pos" };
    const inline std::string OPT_ROT{ "rot" };
    const inline std::string OPT_SCALE{ "scale" };

    struct CreateOptions {
        ki::type_id typeId;
        ki::node_id nodeId;
        ki::node_id parentId{ 0 };
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
                opt.typeId = value.as<unsigned int>();
            }
            else if (k == OPT_NODE) {
                opt.nodeId = value.as<unsigned int>();
            }
            else if (k == OPT_PARENT) {
                opt.parentId = value.as<unsigned int>();
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
                util::degreesToQuat(opt.rot) };

            nodeId = builder.build(opt.parentId, type, state);
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

            CreateState state{
                pos,
                scale,
                quat };

            CompositeBuilder builder{ NodeRegistry::get() };
            if (builder.build(parentId, type, state)) {
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
