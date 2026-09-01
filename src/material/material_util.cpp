#include "material_util.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <tuple>

#include <cmath>
#include <vector>
#include <cstdint>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

namespace
{
    const std::regex CONTAINS_BUILD = std::regex(".*_build.*");

    //
    // Official convert to SRGB from linear colorspace
    // 
    // @see IEC 61966-2-1:1999 -color standard[W3C sRGB Color Standard Specification]
    //
    float linearToSRGB(float linear)
    {
        if (linear <= 0.0031308f) {
            return linear * 12.92f;
        }
        else {
            return 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
        }
    }
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

    std::vector<uint8_t> generateSRGBPixelBuffer(
        const glm::vec4& linearColor,
        int width,
        int height)
    {
        // Muunnetaan R, G ja B kanavat sRGB-avaruuteen. Alpha (A) pidetään aina lineaarisena!
        uint8_t srgbR = static_cast<uint8_t>(glm::clamp(linearToSRGB(linearColor.r) * 255.0f, 0.0f, 255.0f));
        uint8_t srgbG = static_cast<uint8_t>(glm::clamp(linearToSRGB(linearColor.g) * 255.0f, 0.0f, 255.0f));
        uint8_t srgbB = static_cast<uint8_t>(glm::clamp(linearToSRGB(linearColor.b) * 255.0f, 0.0f, 255.0f));
        uint8_t rawA = static_cast<uint8_t>(glm::clamp(linearColor.a * 255.0f, 0.0f, 255.0f));

        // Luodaan 4-kanavainen tavupuskuri taulukkolatausta varten
        std::vector<uint8_t> pixelData(width * height * 4);
        for (int i = 0; i < width * height; ++i) {
            pixelData[i * 4 + 0] = srgbR;
            pixelData[i * 4 + 1] = srgbG;
            pixelData[i * 4 + 2] = srgbB;
            pixelData[i * 4 + 3] = rawA;
        }

        return pixelData;
    }

    std::vector<uint16_t> generateSRGBPixelBuffer16(
        const glm::vec4& linearColor,
        int width,
        int height)
    {
        // Muunnetaan R, G ja B kanavat sRGB-avaruuteen. Alpha (A) pidetään aina lineaarisena.
        // Skaalataan arvo välille 0.0 - 65535.0f (16-bit unsigned short maksimi)
        uint16_t srgbR = static_cast<uint16_t>(glm::clamp(linearToSRGB(linearColor.r) * 65535.0f, 0.0f, 65535.0f));
        uint16_t srgbG = static_cast<uint16_t>(glm::clamp(linearToSRGB(linearColor.g) * 65535.0f, 0.0f, 65535.0f));
        uint16_t srgbB = static_cast<uint16_t>(glm::clamp(linearToSRGB(linearColor.b) * 65535.0f, 0.0f, 65535.0f));
        uint16_t rawA = static_cast<uint16_t>(glm::clamp(linearColor.a * 65535.0f, 0.0f, 65535.0f));

        // Luodaan 4-kanavainen 16-bittinen puskuri (leveys * korkeus * 4 kanavaa)
        std::vector<uint16_t> pixelData(width * height * 4);
        for (int i = 0; i < width * height; ++i) {
            pixelData[i * 4 + 0] = srgbR;
            pixelData[i * 4 + 1] = srgbG;
            pixelData[i * 4 + 2] = srgbB;
            pixelData[i * 4 + 3] = rawA;
        }

        return pixelData;
    }

    // Muuntaa dynaamisen lineaarisen puskurin sRGB RGBA8 -tavuiksi
    std::vector<uint8_t> convertLinearToSRGBBuffer(
        const std::vector<glm::vec4>& linearPixels)
    {
        std::vector<uint8_t> srgbBytes;
        srgbBytes.reserve(linearPixels.size() * 4);

        for (const auto& color : linearPixels) {
            // R, G, B kanavat gammakorjataan, Alpha (A) pidetään aina lineaarisena raakadatana!
            uint8_t r = static_cast<uint8_t>(glm::clamp(linearToSRGB(color.r) * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(glm::clamp(linearToSRGB(color.g) * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(glm::clamp(linearToSRGB(color.b) * 255.0f, 0.0f, 255.0f));
            uint8_t a = static_cast<uint8_t>(glm::clamp(color.a * 255.0f, 0.0f, 255.0f));

            srgbBytes.push_back(r);
            srgbBytes.push_back(g);
            srgbBytes.push_back(b);
            srgbBytes.push_back(a);
        }

        return srgbBytes;
    }
}
