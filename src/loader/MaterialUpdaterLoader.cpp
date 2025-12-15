#include "MaterialUpdaterLoader.h"

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"

#include "ki/sid.h"

#include "mesh/MeshFlags.h"

#include "material/MaterialUpdater.h"
#include "material/ShaderMaterialUpdater.h"
#include "material/FontAtlasMaterialUpdater.h"

#include "loader/document.h"
#include "Loaders.h"
#include "loader_util.h"

namespace loader {
    MaterialUpdaterLoader::MaterialUpdaterLoader(
        const std::shared_ptr<Context>& ctx)
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
                else if (type == "shader") {
                    data.type = MaterialUpdaterType::shader;
                }
                else if (type == "font_atlas") {
                    data.type = MaterialUpdaterType::font_atlas;
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
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else {
                reportUnknown("updater_entry", k, v);
            }
        }
    }

    std::vector<std::unique_ptr<MaterialUpdater>> MaterialUpdaterLoader::createMaterialUpdaters(
        const std::vector<MaterialUpdaterData>& updatersData,
        Loaders& loaders)
    {
        std::vector<std::unique_ptr<MaterialUpdater>> updaters;
        for (const auto& data : updatersData) {
            auto updater = createMaterialUpdater(data, loaders);
            if (!updater) continue;
            updaters.push_back(std::move(updater));
        }

        return updaters;
    }

    std::unique_ptr<MaterialUpdater> MaterialUpdaterLoader::createMaterialUpdater(
        const MaterialUpdaterData& data,
        Loaders& loaders)
    {
        switch (data.type) {
        case MaterialUpdaterType::shader: {
            auto cm = std::make_unique<ShaderMaterialUpdater>(SID(data.id), data.id);

            cm->m_size = data.size;
            cm->m_frameSkip = data.frameSkip;
            cm->setMaterial(&data.materialData.material);
            cm->m_material->loadTextures();

            loaders.m_materialLoader.resolveMaterial({}, *cm->m_material);

            return cm;
        }
        case MaterialUpdaterType::font_atlas: {
            auto cm = std::make_unique<FontAtlasMaterialUpdater>(SID(data.id), data.id);

            cm->setMaterial(&data.materialData.material);

            loaders.m_materialLoader.resolveMaterial({}, *cm->m_material);

            return cm;
        }
        }

        return nullptr;
    }
}
