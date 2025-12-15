#include "CustomMaterialLoader.h"

#include "util/util.h"

#include "mesh/MeshFlags.h"

#include "loader/document.h"
#include "loader_util.h"
#include "loader/Loaders.h"

namespace loader {
    CustomMaterialLoader::CustomMaterialLoader(
        const std::shared_ptr<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void CustomMaterialLoader::loadCustomMaterial(
        const loader::DocNode& node,
        CustomMaterialData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = CustomMaterialType::none;
                }
                else if (type == "skybox") {
                    data.type = CustomMaterialType::skybox;
                }
                else {
                    reportUnknown("custom_material_type", k, v);
                }
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else {
                reportUnknown("custom_material_entry", k, v);
            }
        }
    }

    std::unique_ptr<CustomMaterial> CustomMaterialLoader::createCustomMaterial(
        const CustomMaterialData& data,
        Loaders& loaders)
    {
        return nullptr;
    }
}
