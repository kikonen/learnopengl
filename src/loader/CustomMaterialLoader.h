#pragma once

#include <vector>

#include "material/CustomMaterial.h"

#include "BaseLoader.h"
#include "CustomMaterialData.h"

namespace loader {
    class CustomMaterialLoader : public BaseLoader
    {
    public:
        CustomMaterialLoader(
            Context ctx);

        void loadCustomMaterial(
            const loader::DocNode& node,
            CustomMaterialData& data) const;

        std::unique_ptr<CustomMaterial> createCustomMaterial(
            const CustomMaterialData& data,
            const int cloneIndex,
            const glm::uvec3& tile);

    };
}
