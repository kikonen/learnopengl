#include "SceneAPI.h"

#include <glm/glm.hpp>

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "registry/NodeRegistry.h"

namespace {
    const inline std::string OPT_TYPE{ "type" };
    const inline std::string OPT_PARENT{ "parent" };
    const inline std::string OPT_POS{ "pos" };
    const inline std::string OPT_ROT{ "rot" };
    const inline std::string OPT_SCALE{ "scale" };
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
        auto& nodeRegistry = NodeRegistry::get();

        std::string typeName;
        ki::node_id parentId = 0;
        auto pos = glm::vec3(0);
        auto rot = glm::vec3(0);
        auto scale = glm::vec3(1.f);

        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == OPT_TYPE) {
                typeName = value.as<std::string>();
            }
            else if (k == OPT_PARENT) {
                parentId = value.as<unsigned int>();
            }
            else if (k == OPT_POS) {
                pos = value.as<glm::vec3>();
            }
            else if (k == OPT_ROT) {
                rot = value.as<glm::vec3>();
            }
            else if (k == OPT_SCALE) {
                scale = value.as<glm::vec3>();
            }
            });

        const auto typeId = SID(typeName);
        const auto* type = pool::TypeHandle::toType(typeId);

        const ki::node_id nodeId = ki::StringID::nextID(typeName);

        if (pool::NodeHandle::toHandle(nodeId)) {
            return 0;
        }

        const auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();

        node->m_typeHandle = type->toHandle();

        CreateState state{
            pos,
            scale,
            util::degreesToQuat(rot) };

        nodeRegistry.attachNode(
            nodeId,
            parentId,
            state);

        return nodeId;
    }
}
