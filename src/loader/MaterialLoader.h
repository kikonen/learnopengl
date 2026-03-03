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
            const std::shared_ptr<Context>& ctx);

        void loadMaterialModifiers(
            const loader::DocNode& node,
            const std::string& currentDir,
            std::vector<MaterialData>& materials,
            Loaders& loaders) const;

        void loadMaterialModifier(
            const loader::DocNode& node,
            const std::string& currentDir,
            MaterialData& data,
            Loaders& loaders) const;

        void loadMaterials(
            const loader::DocNode& node,
            const std::string& currentDir,
            std::vector<MaterialData>& materials,
            Loaders& loaders) const;

        void loadMaterial(
            const loader::DocNode& node,
            const std::string& currentDir,
            MaterialData& data,
            Loaders& loaders) const;

        void loadMaterialPrefab(
            const loader::DocNode& node,
            const std::string& currentDir,
            MaterialData& data,
            Loaders& loaders) const;

        void loadMaterialSet(
            const loader::DocNode& node,
            const std::string& currentDir,
            std::vector<MaterialData>& materials,
            Loaders& loaders) const;

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
            const std::map<MaterialProgramType, std::string> programs,
            const std::string& defaultValue = "");
    };
}
