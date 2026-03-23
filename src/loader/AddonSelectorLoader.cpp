#include "AddonSelectorLoader.h"

#include "asset/Assets.h"

#include "util/glm_util.h"
#include "util/util.h"

#include "component/definition/AddonSelectorDefinition.h"

#include "loader/document.h"
#include "loader_util.h"

#include "AddonSelectorData.h"

namespace loader
{
    AddonSelectorLoader::AddonSelectorLoader(
        const std::shared_ptr<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void AddonSelectorLoader::loadAddonSelector(
        const loader::DocNode& node,
        AddonSelectorData& data) const
    {
        const auto& assets = Assets::get();

        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
            }
            else if (k == "seed") {
                data.seed = readInt(v);
            }
            else if (k == "addons") {
                loadAddons(v, data.addons);
            }
            else {
                reportUnknown("addon_selector_entry", k, v);
            }
        }
    }

    void AddonSelectorLoader::loadAddons(
        const loader::DocNode& node,
        std::vector<AddonData>& addons) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = addons.emplace_back();
            loadAddon(entry, data);
        }
    }

    void AddonSelectorLoader::loadAddon(
        const loader::DocNode& node,
        AddonData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "id") {
                data.id = readString(v);
            }
            else if (k == "xid") {
                data.id = readString(v);
                data.enabled = false;
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "group") {
                data.group = readString(v);
            }
            else if (k == "seed") {
                data.seed = readInt(v);
            }
            else if (k == "range") {
                data.range = readUVec2(v);
                data.enabled = false;
            }
        }
    }

    std::unique_ptr<AddonSelectorDefinition> AddonSelectorLoader::createDefinition(
        const AddonSelectorData& data)
    {
        if (!data.enabled) return nullptr;

        auto definition = std::make_unique<AddonSelectorDefinition>();
        auto& df = *definition;

        for (auto& addonData : data.addons) {
            auto& addon = df.addons.emplace_back();
            addon.enabled = addonData.enabled;
            addon.id = SID(addonData.id);
            addon.group = SID(addonData.group);
            addon.seed = addonData.seed;
            addon.range = addonData.range;
        }

        return definition;
    }
}
