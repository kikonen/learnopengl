#pragma once

#include <vector>

#include "BaseLoader.h"
#include "DecalData.h"

namespace loader {
    class DecalLoader : public BaseLoader
    {
    public:
        DecalLoader(
            std::shared_ptr<Context> ctx);

        void loadDecals(
            const loader::DocNode& node,
            std::vector<loader::DecalData>& decals,
            Loaders& loaders) const;

        void loadDecal(
            const loader::DocNode& node,
            loader::DecalData& data,
            Loaders& loaders) const;

        void createDecals(
            const std::vector<loader::DecalData>& decals,
            Loaders& loaders) const;

        void createDecal(
            const loader::DecalData& data,
            Loaders& loaders) const;

    };
}
