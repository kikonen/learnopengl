#pragma once

#include <vector>

#include "BaseLoader.h"
#include "MaterialData.h"

namespace loader {
    class MaterialLoader : public BaseLoader
    {
    public:
        MaterialLoader(
            Context ctx);

        void loadMaterialModifiers(
            const YAML::Node& node,
            MaterialData& data) const;

        void loadMaterials(
            const YAML::Node& node,
            std::vector<MaterialData>& materials) const;

        void loadMaterial(
            const YAML::Node& node,
            MaterialData& data) const;

        void loadMaterialPbr(
            const std::string& pbrName,
            MaterialData& data) const;

        void loadTextureSpec(
            const YAML::Node& node,
            TextureSpec& textureSpec) const;

        void loadTextureWrap(
            const std::string& k,
            const YAML::Node& v,
            GLint& wrapMode) const;

        void modifyMaterial(
            Material& m,
            const MaterialData& data);

    };
}
