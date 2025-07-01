#pragma once

#include <vector>

#include "BaseLoader.h"

struct NodeDefinition;

namespace loader {
    struct NodeData;

    class NodeLoader : public BaseLoader
    {
    public:
        NodeLoader(
            std::shared_ptr<Context> ctx);

        void loadNodes(
            const loader::DocNode& node,
            std::vector<NodeData>& nodes,
            Loaders& loaders) const;

        void loadNode(
            const loader::DocNode& node,
            NodeData& nodeData,
            Loaders& loaders) const;

        void loadNodeClone(
            const loader::DocNode& node,
            NodeData& nodeData,
            bool recurse,
            Loaders& loaders) const;

        void createNodeDefinitions(
            const std::vector<NodeData>& nodes,
            std::vector<NodeDefinition>& definitions);

        void createNodeDefinition(
            const NodeData& node,
            NodeDefinition& definition,
            bool recurse);
    };
}
