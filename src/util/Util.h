#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>
#include <filesystem>


namespace util
{
    //
    // modify str param to upper case
    //
    // @return reference of str param
    inline std::string& toUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    //
    // modify str param to lower case
    //
    // @return reference of str param
    inline std::string& toLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    inline const std::string dirName(const std::string& filename)
    {
        std::filesystem::path p{ filename };
        return p.parent_path().string();
    }

    // https://stackoverflow.com/questions/11421432/how-can-i-output-the-value-of-an-enum-class-in-c11
    template <typename Enumeration>
    auto as_integer(Enumeration const value)
        -> typename std::underlying_type<Enumeration>::type
    {
        return static_cast<typename std::underlying_type<Enumeration>::type>(value);
    }
}
