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

    std::unique_ptr<CompositeDefinition> CompositeLoader::createCompositeDefinition(
        const CompositeData& compositeData)
    {
        if (!compositeData.nodes) return nullptr;

        auto definition = std::make_unique<CompositeDefinition>();
        auto& df = *definition;

        df.m_nodes = std::make_unique<std::vector<NodeDefinition>>();
        createNodeDefinitions(*compositeData.nodes, *df.m_nodes);

        return definition;
    }

    void CompositeLoader::createNodeDefinitions(
        const std::vector<NodeData>& nodes,
        std::vector<NodeDefinition>& definitions)
    {
        for (const auto& nodeData : nodes) {
            auto& definition = definitions.emplace_back();
            createNodeDefinition(nodeData, definition, true);
        }
    }

    void CompositeLoader::createNodeDefinition(
        const NodeData& nodeData,
        NodeDefinition& definition,
        bool recurse)
    {
        auto& df = definition;

        df.m_enabled = nodeData.enabled;
        df.m_id = nodeData.baseId.m_path;
        df.m_aliasId = nodeData.aliasBaseId.m_path;
        df.m_typeId = SID(nodeData.typeId.m_path);
        df.m_ignoredById = nodeData.ignoredByBaseId.m_path;

        df.m_position = nodeData.position;
        df.m_rotation = nodeData.rotation;
        df.m_scale = nodeData.scale;

        df.m_tiling = { nodeData.tilingX, nodeData.tilingY };

        {
            const auto& src = nodeData.repeat;
            auto& dst = df.m_repeat;

            dst.m_xCount = src.xCount;
            dst.m_yCount = src.yCount;
            dst.m_zCount = src.zCount;

            dst.m_xStep = src.xStep;
            dst.m_yStep = src.yStep;
            dst.m_zStep = src.zStep;
        }

        df.m_clonePositionOffset = nodeData.clonePositionOffset;

        if (nodeData.clones && recurse) {
            df.m_clones = std::make_shared<std::vector<NodeDefinition>>();

            for (const auto& cloneData : *nodeData.clones) {
                auto& cloneDefinition = df.m_clones->emplace_back();
                createNodeDefinition(cloneData, cloneDefinition, false);
            }
        }

        if (nodeData.children) {
            df.m_children= std::make_shared<std::vector<NodeDefinition>>();

            for (const auto& childData : *nodeData.children) {
                auto& cloneDefinition = df.m_children->emplace_back();
                createNodeDefinition(childData, cloneDefinition, true);
            }
        }
    }
}
