#pragma once

#include <vector>

#include "asset/CustomMaterial.h"

#include "BaseLoader.h"

namespace loader {
    enum class CustomMaterialType {
        none,
        text,
        skybox,
    };

    struct CustomMaterialData {
        CustomMaterialType type{ CustomMaterialType::none };

        std::string fontName;
        float fontSize;
    };

    class CustomMaterialLoader : public BaseLoader
    {
    public:
        CustomMaterialLoader(
            Context ctx);

        void loadCustomMaterial(
            const YAML::Node& node,
            CustomMaterialData& data);

        std::unique_ptr<CustomMaterial> createCustomMaterial(
            const CustomMaterialData& data,
            const int cloneIndex,
            const glm::uvec3& tile);

    };
}
