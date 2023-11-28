#pragma once

#include <vector>

#include "asset/Material.h"
#include "asset/MaterialField.h"

#include "BaseLoader.h"

namespace loader {
    struct MaterialData
    {
        bool enabled{ false };
        MaterialField fields;
        Material material;
    };

    class MaterialLoader : public BaseLoader
    {
    public:
        MaterialLoader(
            Context ctx);

        void loadMaterialModifiers(
            const YAML::Node& node,
            MaterialData& data) const;

        void loadMaterials(
            const YAML::Node& doc);

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

        Material* find(
            std::string_view name);

    public:
        Material m_defaultMaterial;

    private:
        std::vector<MaterialData> m_materials;
    };
}
