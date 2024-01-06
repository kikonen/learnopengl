#include "CustomMaterialLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "text/TextMaterial.h"

namespace loader {
    CustomMaterialLoader::CustomMaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CustomMaterialLoader::loadCustomMaterial(
        const YAML::Node& node,
        CustomMaterialData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = CustomMaterialType::none;
                }
                else if (type == "skybox") {
                    data.type = CustomMaterialType::skybox;
                }
                else if (type == "text") {
                    data.type = CustomMaterialType::text;
                }
                else {
                    reportUnknown("custom_material_type", k, v);
                }
            }
            else if (k == "font") {
                data.font = readString(v);
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

        //switch (data.type) {
        //case CustomMaterialType::text: {
        //    auto material{ std::make_unique<TextMaterial>() };
        //    material->m_atlas.m_fontName = data.fontName;
        //    material->m_atlas.m_fontSize = data.fontSize;

        //    return material;
        //}
        //}

        return nullptr;
    }

}
