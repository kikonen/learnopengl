#pragma once

#include <vector>

#include "BaseLoader.h"
#include "NodeRoot.h"

namespace loader {
    class NodeLoader : public BaseLoader
    {
    public:
        NodeLoader(
            std::shared_ptr<Context> ctx);

        void loadNodes(
            const loader::DocNode& node,
            std::vector<NodeRoot>& nodes,
            Loaders& loaders) const;

        void loadNode(
            const loader::DocNode& node,
            NodeRoot& nodeRoot,
            Loaders& loaders) const;

        void loadNodeClone(
            const loader::DocNode& node,
            NodeData& nodeData,
            std::vector<NodeData>& clones,
            bool recurse,
            Loaders& loaders) const;

        void loadPrefab(
            const loader::DocNode& node,
            NodeData& data,
            Loaders& loaders) const;

        void loadAttachments(
            const loader::DocNode& node,
            std::vector<AttachmentData>& attachments) const;

        void loadAttachment(
            const loader::DocNode& node,
            AttachmentData& data) const;
    };
}
