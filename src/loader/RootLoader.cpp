#include "RootLoader.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader
{
    RootLoader::RootLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void RootLoader::loadRoot(
        const loader::DocNode& node,
        RootData& data) const
    {
        const auto& assets = Assets::get();

        data.rootId = assets.rootId;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "pos") {
                data.pos = readVec3(v);
            }
        }
    }

    void RootLoader::attachRoot(
        const RootData& data)
    {
        const auto& assets = Assets::get();

        std::string name = "<root>";
        auto typeHandle = pool::TypeHandle::allocate(SID(name));
        auto* type = typeHandle.toType();
        type->setName(name);

        auto& flags = type->m_flags;
        flags.invisible = true;

        auto handle = pool::NodeHandle::allocate(assets.rootId);
        auto* node = handle.toNode();

        node->setName("<root>");
        node->m_typeHandle = typeHandle;
        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        {
            CreateState state{};
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.state = state;
            evt.body.node = {
                .target = data.rootId,
            };
            assert(evt.body.node.target == 1);
            m_dispatcher->send(evt);
        }
    }
}
