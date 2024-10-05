#include "MaterialUpdaterLoader.h"

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/Util.h"

#include "mesh/MeshFlags.h"

#include "material/MaterialUpdater.h"
#include "material/FrameBufferMaterial.h"
#include "material/MaterialRegistry.h"

#include "loader/document.h"
#include "Loaders.h"
#include "loader_util.h"

namespace loader {
    MaterialUpdaterLoader::MaterialUpdaterLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void MaterialUpdaterLoader::loadMaterialUpdater(
        const loader::DocNode& node,
        MaterialUpdaterData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = MaterialUpdaterType::none;
                }
                else if (type == "framebuffer") {
                    data.type = MaterialUpdaterType::framebuffer;
                }
                else {
                    reportUnknown("updater_type", k, v);
                }
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterialModifiers(v, data.materialData, loaders);
            }
            else {
                reportUnknown("updater_entry", k, v);
            }
        }
    }

    std::shared_ptr<MaterialUpdater> MaterialUpdaterLoader::createMaterialUpdater(
        const MaterialUpdaterData& data,
        Loaders& loaders)
    {
        switch (data.type) {
        case MaterialUpdaterType::framebuffer: {
            auto cm = std::make_unique<FrameBufferMaterial>();

            cm->setMaterial(&data.materialData.material);
            cm->m_material->loadTextures();

            loaders.m_materialLoader.resolveMaterial({}, *cm->m_material);
            MaterialRegistry::get().registerMaterial(*cm->m_material);

            return cm;
        }
        }

        return nullptr;
    }
}
