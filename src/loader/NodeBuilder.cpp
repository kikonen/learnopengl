#include "NodeBuilder.h"

#include <string>
#include <vector>
#include <fstream>
#include <span>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "registry/Registry.h"

#include <engine/AsyncLoader.h>

#include "Loaders.h"
#include "Context.h"
#include "NodeData.h"

#include "model/CompositeBuilder.h"
#include "model/NodeDefinition.h"
#include "model/ResolvedNode.h"

#include "SceneLoader.h"
#include "Loaders.h"

#include "loader_util.h"

namespace
{
    const ki::node_id* findAlias(
        loader::BaseId aliasId,
        std::vector<std::pair<std::string, ki::node_id>>& aliases)
    {
        const auto& it = std::find_if(
            aliases.cbegin(),
            aliases.cend(),
            [&aliasId](const auto& e) { return e.first == aliasId.m_path; });
        return it != aliases.end() ? &(*it).second : nullptr;
    }
}

namespace loader
{
    NodeBuilder::NodeBuilder(
        SceneLoader* sceneLoader)
        : m_sceneLoader{ sceneLoader }
    { }

    NodeBuilder::~NodeBuilder() = default;

    void NodeBuilder::loadedNode(
        const NodeData& nodeData,
        bool success)
    {
        m_sceneLoader->loadedNode(nodeData, success);
    }

    void NodeBuilder::addResolvedNode(
        const ResolvedNode& resolved)
    {
        std::lock_guard lock(m_resolvedNodesLock);
        m_resolvedNodes.push_back(resolved);
    }

    // TODO KI need to change node add logic to happen via commands
    // => i.e. api which can be used also from Lua
    bool NodeBuilder::resolveNode(
        const ki::node_id parentId,
        const NodeData& baseData,
        std::shared_ptr<Context> ctx,
        std::shared_ptr<Loaders> loaders)
    {
        if (!baseData.enabled) {
            return false;
        }

        ctx->m_asyncLoader->addLoader(ctx->m_alive, [this, parentId, &baseData, loaders]() {
            std::vector<std::pair<std::string, ki::node_id>> aliases;

            NodeDefinition nodeDefinition;
            loaders->m_nodeLoader.createNodeDefinition(baseData, nodeDefinition, true);

            CompositeBuilder builder{ NodeRegistry::get() };
            builder.buildNode(parentId, nodeDefinition, aliases, true);
            for (auto& resolvedNode : builder.getResolvedNodes()) {
                addResolvedNode(resolvedNode);
            }

            loadedNode(baseData, true);
        });

        return true;
    }
}
