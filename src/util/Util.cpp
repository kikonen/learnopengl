#include "Util.h"

#include <iostream>
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

    const std::string dirName(const std::string& filename)
    {
        std::filesystem::path p{ filename };
        return p.parent_path().string();
    }
}
