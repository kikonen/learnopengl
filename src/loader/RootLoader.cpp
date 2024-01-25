#include "RootLoader.h"

#include "ki/yaml.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"

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
        data.rootId = m_assets.rootId;

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
        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<root>");

        auto& flags = type->m_flags;
        flags.invisible = true;

        auto handle = pool::NodeHandle::allocate(m_ctx.m_assets.rootId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        node->m_resolvedSID = "<root>";
#endif
        node->m_typeHandle = typeHandle;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = data.rootId,
            };
            m_dispatcher->send(evt);
        }
    }
}
