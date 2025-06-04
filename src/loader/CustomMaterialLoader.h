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
            std::shared_ptr<Context> ctx);

        void loadCustomMaterial(
            const loader::DocNode& node,
            CustomMaterialData& data,
            Loaders& loaders) const;

        std::unique_ptr<CustomMaterial> createCustomMaterial(
            const CustomMaterialData& data,
            Loaders& loaders);

    };
}
