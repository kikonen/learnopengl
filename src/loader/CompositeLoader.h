#pragma once

#include <vector>

#include "BaseLoader.h"

namespace model
{
    struct CompositeDefinition;
    struct NodeDefinition;
}

namespace loader {
    class Loaders;

    struct CompositeData;
    struct NodeData;

    class CompositeLoader : public BaseLoader
    {
    public:
        CompositeLoader(
            const std::shared_ptr<Context>& ctx);

        void loadComposites(
            const loader::DocNode& node,
            std::vector<CompositeData>& composites,
            Loaders& loaders) const;

        void loadComposite(
            const loader::DocNode& node,
            CompositeData& data,
            Loaders& loaders) const;

        std::unique_ptr<model::CompositeDefinition> createCompositeDefinition(
            const CompositeData& compositeData,
            Loaders& loaders) const;
    };
}
