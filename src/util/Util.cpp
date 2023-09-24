#include "Util.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace util {
    bool isBool(std::string_view s)
    {
        // TODO KI ...
        return true;
    }

    // https://stackoverflow.com/questions/447206/c-isfloat-function
    bool isInt(std::string_view s)
    {
        int val;
        auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
        return ec == std::errc() && p == s.data() + s.size();
    }

    // https://stackoverflow.com/questions/447206/c-isfloat-function
    bool isFloat(std::string_view s)
    {
        double val;
        auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
        return ec == std::errc() && p == s.data() + s.size();
    }

    std::string& toUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    std::string& toLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    void splitString(
        const std::string& v,
        std::vector<std::string>& result,
        char separator)
    {
        std::istringstream f(v);
        std::string s;
        while (std::getline(f, s, separator)) {
            result.emplace_back(s);
        }
    }

    bool matchAny(
        const std::vector<std::regex>& matchers,
        const std::string& str)
    {
        for (auto& re : matchers) {
            if (std::regex_match(str, re)) {
                return true;
            }
        }
        return false;
    }

    bool fileExists(std::string_view filePath)
    {
        std::ifstream f(std::string{ filePath });
        return f.good();
    }

    std::string dirName(std::string_view filePath)
    {
        std::string path{ filePath };
        std::replace(path.begin(), path.end(), '/', '\\');

        std::filesystem::path p{ path };
        return p.parent_path().string();
    }

    std::string baseName(std::string_view filePath)
    {
        std::string path{ filePath };
        std::replace(path.begin(), path.end(), '/', '\\');

        std::filesystem::path p{ path };
        return p.string();
    }

    std::string joinPath(
        std::string_view rootDir,
        std::string_view parentDir,
        std::string_view baseName,
        std::string_view fileExt)
    {
        std::filesystem::path filePath;

        if (!rootDir.empty()) {
            std::string path{ rootDir };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        if (!parentDir.empty()) {
            std::string path{ parentDir };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        if (!baseName.empty()) {
            std::string path{ baseName };
            if (!fileExt.empty()) {
                path += fileExt;
            }
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        return filePath.string();
    }

    std::string joinPath(
        std::string_view rootDir,
        std::string_view baseName)
    {
        std::filesystem::path filePath;

        if (!rootDir.empty()) {
            std::string path{ rootDir };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        if (!baseName.empty()) {
            std::string path{ baseName };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        return filePath.string();
    }

}
