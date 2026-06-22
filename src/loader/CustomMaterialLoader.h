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
            const util::Ref<Context>& ctx);

        void loadCustomMaterial(
            const loader::DocNode& node,
            const std::string& currentDir,
            CustomMaterialData& data,
            Loaders& loaders) const;

        util::Ref<CustomMaterial> createCustomMaterial(
            const CustomMaterialData& data,
            Loaders& loaders);

    };
}
