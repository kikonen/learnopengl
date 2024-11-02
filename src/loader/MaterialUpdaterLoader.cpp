#include "MaterialUpdaterLoader.h"

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"

#include "ki/sid.h"

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

    void MaterialUpdaterLoader::loadMaterialUpdaters(
        const loader::DocNode& node,
        std::vector<MaterialUpdaterData>& updaters,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = updaters.emplace_back();
            loadMaterialUpdater(entry, data, loaders);
        }
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

            if (k == "id") {
                data.id = readString(v);
            }
            else if (k == "type") {
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
            else if (k == "size") {
                data.size = readVec2(v);
            }
            else if (k == "frame_skip") {
                data.frameSkip = readInt(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterialModifiers(v, data.materialData, loaders);
            }
            else {
                reportUnknown("updater_entry", k, v);
            }
        }
    }

    void MaterialUpdaterLoader::createMaterialUpdaters(
        const std::vector<MaterialUpdaterData>& updaters,
        Loaders& loaders)
    {
        auto& mr = MaterialRegistry::get();
        for (const auto& data : updaters) {
            auto updater = createMaterialUpdater(data, loaders);
            mr.addMaterialUpdater(std::move(updater));
        }
    }

    std::unique_ptr<MaterialUpdater> MaterialUpdaterLoader::createMaterialUpdater(
        const MaterialUpdaterData& data,
        Loaders& loaders)
    {
        switch (data.type) {
        case MaterialUpdaterType::framebuffer: {
            auto cm = std::make_unique<FrameBufferMaterial>(SID(data.id), data.id);

            cm->m_size = data.size;
            cm->m_frameSkip = data.frameSkip;
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
