#pragma once

#include <string>

namespace util
{
    const std::string& toUpper(std::string& str);

    const std::string& toLower(std::string& str);

    // https://stackoverflow.com/questions/11421432/how-can-i-output-the-value-of-an-enum-class-in-c11
    template <typename Enumeration>
    auto as_integer(Enumeration const value)
        -> typename std::underlying_type<Enumeration>::type
    {
        return static_cast<typename std::underlying_type<Enumeration>::type>(value);
    }
}
