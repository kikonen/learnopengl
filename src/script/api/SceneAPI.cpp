#include "SceneAPI.h"

#include <glm/glm.hpp>

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "registry/NodeRegistry.h"

namespace script::api
{
    SceneAPI::SceneAPI()
    {
    }

    SceneAPI::~SceneAPI() = default;

    ki::node_id SceneAPI::lua_create_node(
        const sol::table& opt)
    {
        auto& nodeRegistry = NodeRegistry::get();

        const std::string typeName{ "skeleton_army_{c}_{t}" };

        auto typeId = SID(typeName);

        const auto* type = pool::TypeHandle::toType(typeId);

        auto pos = glm::vec3(0, 0, 0);
        auto rot = glm::vec3(0);
        auto scale = glm::vec3(1);

        ki::node_id parentId = 0;
        ki::node_id nodeId = ki::StringID::nextID();

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        node->m_typeHandle = type->toHandle();
        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        CreateState state;
        state.m_position = pos;
        state.m_scale = scale;
        state.m_rotation = util::degreesToQuat(rot);

        nodeRegistry.attachNode(
            nodeId,
            parentId,
            state);

        return nodeId;
    }
}
