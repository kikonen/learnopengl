#include "Util.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <ranges>
#include <sys/stat.h>

#include <fmt/format.h>

namespace {
    const std::vector<std::regex> BOOL_MATCHERS{
        std::regex("true"),
        std::regex("false"),
        std::regex("yes"),
        std::regex("no"),
        std::regex("1"),
        std::regex("0"),
    };

    const std::vector<std::regex> BOOL_TRUE_MATCHERS{
        std::regex("true"),
        std::regex("yes"),
        std::regex("1"),
    };
}

namespace util {
    bool isBool(std::string_view s)
    {
        std::string str{ s };
        return matchAny(BOOL_MATCHERS, str);
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

    bool readBool(std::string_view s, bool defaultValue)
    {
        std::string str{ s };
        if (!matchAny(BOOL_MATCHERS, str)) return defaultValue;
        return matchAny(BOOL_TRUE_MATCHERS, str);
    }

    std::string toUpper(std::string_view str)
    {
        std::string s{ str };
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    std::string toLower(std::string_view str)
    {
        std::string s{ str };
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    bool startsWith(std::string_view str, std::string_view suffix)
    {
        std::string s{ str };
        return s.rfind(suffix, 0) == 0;
    }

    bool endsWith(std::string_view str, std::string_view suffix)
    {
        std::string s{ str };
        return s.rfind(suffix, s.size()) == s.size();
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

    std::string join(
        const std::string& s1,
        const std::string& s2,
        std::string_view sep)
    {
        if (s1.empty()) return s2;
        if (s2.empty()) return s1;

        return fmt::format("{}{}{}", s1, sep, s2);
    }

    std::string join(
        const std::vector<std::string>& arr,
        std::string_view sep)
    {
        // https://stackoverflow.com/questions/5689003/how-to-implode-a-vector-of-strings-into-a-string-the-elegant-way
        return fmt::format(
            "{}",
            fmt::join(arr, sep));
    }

    std::string replace(
        const std::string& str,
        const std::string_view from,
        const std::string_view to)
    {
        std::string s{ str };

        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return s;
        s.replace(start_pos, from.length(), to);
        return s;
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

    bool fileExists(std::string_view basePath, std::string_view filePath)
    {
        return fileExists(joinPath(basePath, filePath));
    }

    bool fileExists(std::string_view filePath)
    {
        std::ifstream f(std::string{ filePath });
        return f.good();
    }

    bool dirExists(std::string_view filePath)
    {
        // https://www.geeksforgeeks.org/how-to-check-a-file-or-directory-exists-in-cpp/
        // https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
        return std::filesystem::is_directory(filePath);
    }

    std::filesystem::file_time_type fileModifiedAt(std::string_view filePath)
    {
        return std::filesystem::last_write_time(filePath);
    }

    std::string readFile(std::string_view basePath, std::string_view filename)
    {
        std::stringstream buffer;

        std::string filePath = util::joinPath(
            basePath,
            filename);

        if (!util::fileExists(filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
        }

        try {
            std::ifstream t(filePath);
            t.exceptions(std::ifstream::badbit);
            //t.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            buffer << t.rdbuf();
        }
        catch (std::ifstream::failure e) {
            std::string what{ e.what() };
            const auto msg = fmt::format(
                "FILE_NOT_SUCCESFULLY_READ: {}\n{}",
                filePath, what);

            throw std::runtime_error{ msg };
        }
        return buffer.str();
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

    std::string joinPathExt(
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

    std::string joinPathExt(
        std::string_view rootDir,
        std::string_view basePath,
        std::string_view fileExt)
    {
        std::filesystem::path filePath;

        if (!rootDir.empty()) {
            std::string path{ rootDir };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        if (!basePath.empty()) {
            std::string path{ basePath };
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

    std::string joinPath(
        std::vector<std::string_view> parts)
    {
        std::filesystem::path filePath;

        for (const auto& part : parts) {
            if (part.empty()) continue;
            std::string path{ part };
            std::replace(path.begin(), path.end(), '/', '\\');
            filePath /= path;
        }

        return filePath.string();
    }

    std::string appendLineNumbers(const std::string& src)
    {
        std::stringstream sb;

        std::istringstream f{ src };

        int lineNumber = 1;
        std::string line;
        while (std::getline(f, line)) {
            sb << fmt::format("{:<4}: ", lineNumber) << line << "\n";
            lineNumber++;
        }

        return sb.str();
    }

    float prnd(float max)
    {
        // https://stackoverflow.com/questions/686353/random-float-number-generation
        float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / max));
        return r2;
    }
}
