#pragma once

#include "ki/size.h"

namespace loader {
    struct LodData {
        std::string name;

        // -1 == collision mesh
        int8_t level{ 0 };

        float distance{ 0 };
    };
}
