#pragma once

#include <vector>

#include "BaseLoader.h"

#include "NodeTypeData.h"

namespace loader {
    class NodeTypeLoader : public BaseLoader
    {
    public:
        NodeTypeLoader(
            std::shared_ptr<Context> ctx);

        void loadNodeTypes(
            const loader::DocNode& node,
            std::vector<NodeTypeData>& nodeTypes,
            Loaders& loaders) const;

        void loadNodeType(
            const loader::DocNode& node,
            NodeTypeData& nodeType,
            const std::unordered_map<std::string, const loader::DocNode*>& idToType,
            Loaders& loaders) const;

        void loadPrefab(
            const loader::DocNode& node,
            NodeTypeData& data,
            const std::unordered_map<std::string, const loader::DocNode*>& idToType,
            Loaders& loaders) const;

        void loadAttachments(
            const loader::DocNode& node,
            std::vector<AttachmentData>& attachments) const;

        void loadAttachment(
            const loader::DocNode& node,
            AttachmentData& data) const;
    };
}
