#pragma once

#include <string>
#include <string_view>
#include <vector>

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

    // https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
    void splitString(
        const std::string& v,
        std::vector<std::string>& result,
        char separator);

    bool fileExists(std::string filepath);

    std::string dirName(const std::string& filename);

    std::string joinPath(
        const std::string& rootDir,
        const std::string& parentDir,
        const std::string& baseName,
        const std::string& fileExt);

    std::string joinPath(
        const std::string& rootDir,
        const std::string& baseName);

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
