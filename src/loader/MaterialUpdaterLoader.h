#pragma once

#include <vector>
#include <memory>

#include "BaseLoader.h"
#include "MaterialUpdaterData.h"

class MaterialUpdater;

namespace loader {
    class MaterialUpdaterLoader : public BaseLoader
    {
    public:
        MaterialUpdaterLoader(
            const util::Ref<Context>& ctx);

        void loadMaterialUpdaters(
            const loader::DocNode& node,
            const std::string& currentDir,
            std::vector<MaterialUpdaterData>& updaters,
            Loaders& loaders) const;

        void loadMaterialUpdater(
            const loader::DocNode& node,
            const std::string& currentDir,
            MaterialUpdaterData& data,
            Loaders& loaders) const;

        std::vector<util::Ref<MaterialUpdater>> createMaterialUpdaters(
            const std::vector<MaterialUpdaterData>& updaters,
            Loaders& loaders);

        util::Ref<MaterialUpdater> createMaterialUpdater(
            const MaterialUpdaterData& data,
            Loaders& loaders);
    };
}
