#include "Util.h"

#include <algorithm>

namespace util {

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
}
