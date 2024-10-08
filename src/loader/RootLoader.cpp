#include "RootLoader.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader
{
    RootLoader::RootLoader(
        Context ctx)
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

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<root>");

        auto& flags = type->m_flags;
        flags.invisible = true;

        auto handle = pool::NodeHandle::allocate(assets.rootId);
        auto* node = handle.toNode();

        node->setName("<root>");
        node->m_typeHandle = typeHandle;

        {
            NodeState state{};
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
