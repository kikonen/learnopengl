#pragma once

#include "loader/document.h"

namespace loader {
    struct PhysicsCategoryLoader {
        void loadMask(
            const loader::DocNode& node,
            uint32_t& mask) const;

    };
}
