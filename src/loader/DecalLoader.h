#pragma once

#include <vector>

#include "BaseLoader.h"
#include "DecalData.h"

namespace decal
{
    struct DecalDefinition;
}

namespace loader {
    class DecalLoader : public BaseLoader
    {
    public:
        DecalLoader(
            const std::shared_ptr<Context>& ctx);

        void loadDecals(
            const loader::DocNode& node,
            std::vector<loader::DecalData>& decals,
            Loaders& loaders) const;

        void loadDecal(
            const loader::DocNode& node,
            loader::DecalData& data,
            Loaders& loaders) const;

        void loadDecalPrefab(
            const loader::DocNode& node,
            DecalData& data,
            Loaders& loaders) const;

        std::vector<decal::DecalDefinition> createDecals(
            const std::vector<loader::DecalData>& decals,
            Loaders& loaders) const;

        decal::DecalDefinition createDecal(
            const loader::DecalData& data,
            Loaders& loaders) const;
    };
}
