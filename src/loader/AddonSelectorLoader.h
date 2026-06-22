#pragma once

#include "BaseLoader.h"
#include "AddonSelectorData.h"

struct AddonSelectorDefinition;

namespace loader
{
    class AddonSelectorLoader : public BaseLoader
    {
    public:
        AddonSelectorLoader(
            const util::Ref<Context>& ctx);

        void loadAddonSelector(
            const loader::DocNode& node,
            AddonSelectorData& data) const;

        void loadAddons(
            const loader::DocNode& node,
            std::vector<AddonData>& addons) const;

        void loadAddon(
            const loader::DocNode& node,
            AddonData& data) const;

        std::unique_ptr<AddonSelectorDefinition> createDefinition(
            const AddonSelectorData& data);

    };
}
