#include "DecalLoader.h"

#include "util/util.h"

#include "decal/DecalRegistry.h"

#include "loader/document.h"
#include "loader_util.h"
#include "loader/Loaders.h"

namespace loader {
    DecalLoader::DecalLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void DecalLoader::loadDecals(
        const loader::DocNode& node,
        std::vector<loader::DecalData>& decals,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = decals.emplace_back();
            loadDecal(entry, data, loaders);
        }
    }

    void DecalLoader::loadDecal(
        const loader::DocNode& node,
        loader::DecalData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else {
                reportUnknown("decal_entry", k, v);
            }
        }
    }

    void DecalLoader::createDecals(
        const std::vector<loader::DecalData>& decals,
        Loaders& loaders) const
    {
        for (const auto& data : decals) {
            createDecal(data, loaders);
        }
    }

    void DecalLoader::createDecal(
        const loader::DecalData& data,
        Loaders& loaders) const
    {
        decal::DecalDefinition df;
        df.m_sid = SID(data.name);

        decal::DecalRegistry::get().addDecal(df);
    }
}
