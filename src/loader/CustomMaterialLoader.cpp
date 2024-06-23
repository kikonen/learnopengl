#include "CustomMaterialLoader.h"

#include "util/Util.h"

#include "loader/document.h"

namespace loader {
    CustomMaterialLoader::CustomMaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CustomMaterialLoader::loadCustomMaterial(
        const loader::DocNode& node,
        CustomMaterialData& data) const
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
            else {
                reportUnknown("custom_material_entry", k, v);
            }
        }
    }

    std::unique_ptr<CustomMaterial> CustomMaterialLoader::createCustomMaterial(
        const CustomMaterialData& data,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        if (data.type == CustomMaterialType::none) return nullptr;

        return nullptr;
    }

}
