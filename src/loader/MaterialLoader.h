#pragma once

#include <vector>
#include <filesystem>
#include <memory>

#include "BaseLoader.h"
#include "MaterialData.h"

namespace mesh {
    struct MeshFlags;
}

namespace loader {
    class MaterialLoader : public BaseLoader
    {
    public:
        MaterialLoader(
            Context ctx);

        void loadMaterialModifiers(
            const loader::DocNode& node,
            MaterialData& data,
            Loaders& loaders) const;

        void loadMaterials(
            const loader::DocNode& node,
            std::vector<MaterialData>& materials,
            Loaders& loaders) const;

        void loadMaterial(
            const loader::DocNode& node,
            MaterialData& data,
            Loaders& loaders) const;

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
            const loader::DocNode& node,
            TextureSpec& textureSpec) const;

        void loadTextureWrap(
            const std::string& k,
            const loader::DocNode& v,
            uint16_t& wrapMode) const;

        void modifyMaterial(
            Material& m,
            const MaterialData& data);

        void resolveMaterial(
            const mesh::MeshFlags& meshFlags,
            Material& material);

        void resolveProgram(
            const mesh::MeshFlags& meshFlags,
            Material& material);

        std::string selectProgram(
            MaterialProgramType type,
            const std::unordered_map<MaterialProgramType, std::string> programs,
            const std::string& defaultValue = "");
    };
}
