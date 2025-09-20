#pragma once

#include <vector>

#include "BaseLoader.h"

namespace loader {
    struct SceneData;
    struct MetaData;

    class IncludeLoader : public BaseLoader
    {
    public:
        IncludeLoader(
            std::shared_ptr<Context> ctx);

        void loadIncludes(
            const loader::DocNode& node,
            SceneData& sceneData,
            Loaders& loaders) const;

        void loadInclude(
            const loader::DocNode& node,
            SceneData& sceneData,
            Loaders& loaders) const;

        void loadScene(
            const loader::DocNode& node,
            SceneData& sceneData,
            Loaders& loaders) const;

        void loadMeta(
            const loader::DocNode& node,
            MetaData& data) const;
    };
}
