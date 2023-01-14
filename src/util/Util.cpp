#include "Util.h"

#include <algorithm>

namespace util {

    const std::string& toUpper(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    const std::string& toLower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
}
