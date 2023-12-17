#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <regex>


namespace util
{
    bool isBool(std::string_view s);
    bool isInt(std::string_view s);
    bool isFloat(std::string_view s);

    //
    // modify str param to upper case
    //
    // @return reference of str param
    std::string& toUpper(std::string& str);

    //
    // modify str param to lower case
    //
    // @return reference of str param
    std::string& toLower(std::string& str);

    std::string replace(
        const std::string& str,
        const std::string_view from,
        const std::string_view to);

    // https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
    void splitString(
        const std::string& v,
        std::vector<std::string>& result,
        char separator);

    bool matchAny(
        const std::vector<std::regex>& matchers,
        const std::string& str);

    bool fileExists(std::string_view filePath);

    std::string readFile(
        std::string_view basePath,
        std::string_view filename);

    std::string dirName(std::string_view filePath);

    std::string baseName(std::string_view filePath);

    std::string joinPath(
        std::string_view rootDir,
        std::string_view parentDir,
        std::string_view baseName,
        std::string_view fileExt);

    std::string joinPath(
        std::string_view rootDir,
        std::string_view baseName);

    // https://stackoverflow.com/questions/11421432/how-can-i-output-the-value-of-an-enum-class-in-c11
    template <typename Enumeration>
    auto as_integer(Enumeration const value)
        -> typename std::underlying_type<Enumeration>::type
    {
        return static_cast<typename std::underlying_type<Enumeration>::type>(value);
    }

    // https://stackoverflow.com/questions/3998978/using-a-const-key-for-unordered-map
    struct constant_string_hash {
        std::size_t operator () (const std::string& s) const {
            return std::hash<std::string>{}(s);
        }
    };
}
