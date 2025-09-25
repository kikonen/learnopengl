#pragma once

#include <vector>

#include "BaseLoader.h"

namespace model
{
    struct NodeDefinition;
}

namespace loader {
    struct NodeData;
    struct AttachmentData;

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

        void loadAttachment(
            const loader::DocNode& node,
            AttachmentData& data) const;

        void createNodeDefinitions(
            const std::vector<NodeData>& nodes,
            std::vector<model::NodeDefinition>& definitions,
            bool recurse) const;

        void createNodeDefinition(
            const NodeData& node,
            model::NodeDefinition& definition,
            bool recurse) const;
    };
}
