#include "CompositeLoader.h"

#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "model/CompositeDefinition.h"
#include "model/NodeDefinition.h"

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader_util.h"
#include "loaders.h"

#include "Context.h"
#include "Loaders.h"

#include "CompositeData.h"
#include "NodeData.h"

namespace loader {
    CompositeLoader::CompositeLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void CompositeLoader::loadComposites(
        const loader::DocNode& node,
        std::vector<CompositeData>& composites,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& compositeData = composites.emplace_back();
            loadComposite(
                entry,
                compositeData,
                loaders);
        }
    }

    void CompositeLoader::loadComposite(
        const loader::DocNode& node,
        CompositeData& compositeData,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "id") {
                compositeData.baseId = readId(v);
            }
            else if (k == "nodes") {
                compositeData.nodes = std::make_shared<std::vector<NodeData>>();

                for (const auto& entry : v.getNodes()) {
                    auto& nodeData = compositeData.nodes->emplace_back();
                    loaders.m_nodeLoader.loadNode(entry, nodeData, loaders);
                }
            }
        }
    }

    std::unique_ptr<model::CompositeDefinition> CompositeLoader::createCompositeDefinition(
        const CompositeData& compositeData,
        Loaders& loaders) const
    {
        if (!compositeData.nodes) return nullptr;

        auto definition = std::make_unique<model::CompositeDefinition>();
        auto& df = *definition;

        df.m_nodes = std::make_unique<std::vector<model::NodeDefinition>>();
        loaders.m_nodeLoader.createNodeDefinitions(*compositeData.nodes, *df.m_nodes, true);

        return definition;
    }
}
