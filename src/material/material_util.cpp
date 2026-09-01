#include "material_util.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <tuple>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

namespace
{
    const std::regex CONTAINS_BUILD = std::regex(".*_build.*");
}

namespace material
{
    ResolvedTexturePath selectTexturePath(
        const std::string& name,
        const std::string& path,
        bool useCompressed)
    {
        const auto& assets = Assets::get();

        std::filesystem::path filePath;

        bool found = false;

        {
            std::filesystem::path buildPath{ path };
            const auto& stem = buildPath.stem().string();

            if (std::regex_match(stem, CONTAINS_BUILD)) {
                buildPath.replace_filename(fmt::format("{}.{}", stem, "png"));
            }
            else {
                buildPath.replace_filename(fmt::format("{}_build.{}", stem, "png"));
            }

            if (useCompressed && assets.compressedTexturesEnabled) {
                std::filesystem::path ktxPath{ buildPath };
                ktxPath.replace_extension(".ktx");

                const auto fullPath = util::joinPath(
                    assets.assetsBuildDir,
                    ktxPath.string());

                if (util::fileExists(fullPath)) {
                    filePath = fullPath;
                    found = true;
                }
            }

            //const auto re = std::regex(".*scenery_build.png");
            //if (std::regex_match(buildPath.string(), re)) {
            //    int x = 0;
            //}

            if (!found) {
                const auto fullPath = util::joinPath(
                    assets.assetsBuildDir,
                    buildPath.string());

                if (util::fileExists(fullPath)) {
                    filePath = fullPath;
                    found = true;
                }
            }
        }

        if (!found) {
            filePath = util::joinPath(assets.assetsDir, path);
        }

        return { name, filePath.string(), useCompressed, found};
    }

    ResolvedTexturePath resolveTexturePath(
        const std::string& textureName)
    {
        return resolveTexturePath("", "", textureName, false);
    }

    ResolvedTexturePath resolveTexturePath(
        const std::string& baseDir,
        const std::string& modelDir,
        const std::string& textureName,
        bool compressed)
    {
        if (textureName.empty()) return {};

        const auto& assets = Assets::get();

        ResolvedTexturePath texturePath{ "", "", false, false };

        if (!baseDir.empty()) {
            // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
            texturePath = selectTexturePath(
                textureName,
                util::joinPathExt(
                    modelDir,
                    baseDir,
                    textureName,
                    ""),
                compressed);
        }

        if (!texturePath.valid) {
            // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
            texturePath = selectTexturePath(
                textureName,
                util::joinPathExt(
                    modelDir,
                    textureName,
                    ""),
                compressed);
        }

        if (!texturePath.valid && !baseDir.empty()) {
            // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
            texturePath = selectTexturePath(
                textureName,
                util::joinPathExt(
                    baseDir,
                    textureName,
                    ""),
                compressed);
        }

        if (!texturePath.valid && baseDir.empty()) {
            // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
            texturePath = selectTexturePath(
                textureName,
                textureName,
                compressed);
        }

        if (!texturePath.valid) {
            KI_WARN_OUT(fmt::format(
                "TEX::MISSING: base_dir={}, name={}",
                baseDir,
                textureName));
        }
        else {
            KI_INFO_OUT(fmt::format(
                "TEX::FOUND: base_dir={}, name={}, path={}",
                baseDir,
                textureName,
                texturePath.path));
        }

        return texturePath;
    }

    ResolvedTexturePath getPlaceholderTexturePath()
    {
        const auto& assets = Assets::get();
        auto texturePath = resolveTexturePath("", "", assets.placeholderTexture, false);
        texturePath.name = "tex-placeholder";
        return texturePath;
    }
}
