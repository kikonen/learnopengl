#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace util
{
    template<typename T, typename Func>
    std::string join(
        const std::vector<T>& vec,
        const std::string& separator,
        Func toString)
    {
        if (vec.empty()) return "";

        std::ostringstream oss;
        oss << toString(vec[0]);

        for (size_t i = 1; i < vec.size(); ++i) {
            oss << separator << toString(vec[i]);
        }

        return oss.str();
    }
}
