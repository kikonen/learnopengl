#pragma once

#include <vector>

#include "BaseLoader.h"

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
    };
}
