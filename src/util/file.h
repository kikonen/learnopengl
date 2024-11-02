#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <functional>
#include <ranges>
#include <filesystem>

namespace util {
    bool fileExists(std::string_view basePath, std::string_view filePath);
    bool fileExists(std::string_view filePath);
    bool dirExists(std::string_view filePath);

    std::filesystem::file_time_type fileModifiedAt(std::string_view filePath);

    std::string readFile(
        std::string_view basePath,
        std::string_view filename);

    std::string dirName(std::string_view filePath);

    std::string baseName(std::string_view filePath);

    // Join path and fileExt into end
    std::string joinPathExt(
        std::string_view rootDir,
        std::string_view parentDir,
        std::string_view baseName,
        std::string_view fileExt);

    std::string joinPathExt(
        std::string_view rootDir,
        std::string_view parentDir,
        std::string_view basePath,
        std::string_view baseName,
        std::string_view fileExt);

    // Join path and fileExt into end
    std::string joinPathExt(
        std::string_view rootDir,
        std::string_view basePath,
        std::string_view fileExt);

    std::string joinPath(
        std::string_view rootDir,
        std::string_view baseName);

    std::string joinPath(
        std::vector<std::string_view> paths);

}
