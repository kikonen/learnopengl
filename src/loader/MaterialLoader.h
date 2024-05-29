#pragma once

#include <vector>
#include <filesystem>

#include "BaseLoader.h"
#include "MaterialData.h"

namespace loader {
    class MaterialLoader : public BaseLoader
    {
    public:
        MaterialLoader(
            Context ctx);

        void loadMaterialModifiers(
            const loader::Node& node,
            MaterialData& data) const;

        void loadMaterials(
            const loader::Node& node,
            std::vector<MaterialData>& materials) const;

        void loadMaterial(
            const loader::Node& node,
            MaterialData& data) const;

        void resolveMaterialPaths(
            const std::string& baseDir,
            MaterialData& data) const;

        void resolveMaterialPbr(
            const std::string& baseDir,
            MaterialData& data) const;

        void loadMaterialPbr(
            const std::string& baseDir,
            const std::string& pbrName,
            MaterialData& data) const;

        bool handlePbrEntry(
            const std::string& baseDir,
            const std::string& pbrName,
            const std::filesystem::directory_entry& dirEntry,
            MaterialData& data) const;

        void loadTextureSpec(
            const loader::Node& node,
            TextureSpec& textureSpec) const;

        void loadTextureWrap(
            const std::string& k,
            const loader::Node& v,
            GLint& wrapMode) const;

        void modifyMaterial(
            Material& m,
            const MaterialData& data);

    };
}
