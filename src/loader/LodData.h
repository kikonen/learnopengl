#pragma once

#include "ki/size.h"

#include "FlagContainer.h"

namespace loader {
    struct LodData {
        std::string name;

        // -1 == collision mesh
        int8_t level{ 0 };
        int8_t priority{ 0 };

        float distance{ 0 };

        loader::FlagContainer meshFlags;
    };
}
