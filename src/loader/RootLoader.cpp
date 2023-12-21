#include "RootLoader.h"

#include "ki/yaml.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"

namespace loader
{
    RootLoader::RootLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void RootLoader::loadRoot(
        const YAML::Node& node,
        RootData& data) const
    {
        data.rootId = m_assets.rootUUID;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "pos") {
                data.pos = readVec3(v);
            }
        }
    }

    void RootLoader::attachRoot(
        const RootData& data)
    {
        auto* type = m_registry->m_typeRegistry->registerType("<root>");
        type->m_entityType = EntityType::origo;

        auto& flags = type->m_flags;
        flags.invisible = true;

        auto node = new Node(type);
        node->m_uuid = data.rootId;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            m_dispatcher->send(evt);
        }
    }
}
