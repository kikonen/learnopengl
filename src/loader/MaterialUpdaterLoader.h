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
            std::shared_ptr<Context> ctx);

        void loadMaterialUpdaters(
            const loader::DocNode& node,
            std::vector<MaterialUpdaterData>& updaters,
            Loaders& loaders) const;

        void loadMaterialUpdater(
            const loader::DocNode& node,
            MaterialUpdaterData& data,
            Loaders& loaders) const;

        void createMaterialUpdaters(
            const std::vector<MaterialUpdaterData>& updaters,
            Loaders& loaders);

        std::unique_ptr<MaterialUpdater> createMaterialUpdater(
            const MaterialUpdaterData& data,
            Loaders& loaders);
    };
}
