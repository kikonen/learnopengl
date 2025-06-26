#pragma once

#include <vector>

#include "BaseLoader.h"

struct CompositeDefinition;
struct NodeDefinition;

namespace loader {
    struct CompositeData;
    struct NodeData;

    class CompositeLoader : public BaseLoader
    {
    public:
        CompositeLoader(
            std::shared_ptr<Context> ctx);

        void loadComposites(
            const loader::DocNode& node,
            std::vector<CompositeData>& composites,
            Loaders& loaders) const;

        void loadComposite(
            const loader::DocNode& node,
            CompositeData& data,
            Loaders& loaders) const;

        std::unique_ptr<CompositeDefinition> createCompositeDefinition(
            const CompositeData& compositeData);

    private:
        void createNodeDefinitions(
            const std::vector<NodeData>& nodes,
            std::vector<NodeDefinition>& definitions);

        void createNodeDefinition(
            const NodeData& node,
            NodeDefinition& definition,
            bool recurse);
    };
}
