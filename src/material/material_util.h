#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace material
{
    struct ResolvedTexturePath
    {
        std::string name;
        std::string path;
        bool compressed;
        bool valid;
    };

    ResolvedTexturePath resolveTexturePath(
        const std::string& textureName);

    ResolvedTexturePath resolveTexturePath(
        const std::string& baseDir,
        const std::string& modelDir,
        const std::string& textureName,
        bool compressed);

    ResolvedTexturePath getPlaceholderTexturePath();

    // Generate texture data for ColorTexture
    std::vector<uint8_t> generateSRGBPixelBuffer(
        const glm::vec4& linearColor,
        int width,
        int height);

    // Generate texture data for ColorTexture
    std::vector<uint16_t> generateSRGBPixelBuffer16(
        const glm::vec4& linearColor,
        int width,
        int height);

    std::vector<uint8_t> convertLinearToSRGBBuffer(
        const std::vector<glm::vec4>& linearPixels);
}
