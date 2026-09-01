#pragma once

#include <string>

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
}
