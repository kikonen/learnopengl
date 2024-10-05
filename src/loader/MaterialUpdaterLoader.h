#pragma once

#include <memory>

#include "BaseLoader.h"
#include "MaterialUpdaterData.h"

class MaterialUpdater;

namespace loader {
    class MaterialUpdaterLoader : public BaseLoader
    {
    public:
        MaterialUpdaterLoader(
            Context ctx);

        void loadMaterialUpdater(
            const loader::DocNode& node,
            MaterialUpdaterData& data,
            Loaders& loaders) const;

        std::shared_ptr<MaterialUpdater> createMaterialUpdater(
            const MaterialUpdaterData& data,
            Loaders& loaders);
    };
}
