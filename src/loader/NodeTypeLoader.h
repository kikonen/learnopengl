#pragma once

#include <vector>

#include "BaseLoader.h"

#include "NodeTypeData.h"

namespace loader {
    struct SceneData;

    class NodeTypeLoader : public BaseLoader
    {
    public:
        NodeTypeLoader(
            const std::shared_ptr<Context>& ctx);

        void loadNodeTypes(
            const loader::DocNode& node,
            const std::string& currentDir,
            SceneData& sceneData,
            std::vector<NodeTypeData>& nodeTypes,
            Loaders& loaders) const;

        void loadNodeType(
            const loader::DocNode& node,
            const std::string& currentDir,
            NodeTypeData& nodeType,
            const std::unordered_map<std::string, const loader::DocNode*>& idToType,
            Loaders& loaders) const;

        void loadPrefab(
            const loader::DocNode& node,
            const std::string& currentDir,
            NodeTypeData& data,
            const std::unordered_map<std::string, const loader::DocNode*>& idToType,
            Loaders& loaders) const;

        void loadAddons(
            const loader::DocNode& node,
            std::vector<AddonData>& addons) const;

        void loadAddon(
            const loader::DocNode& node,
            AddonData& data) const;
    };
}
