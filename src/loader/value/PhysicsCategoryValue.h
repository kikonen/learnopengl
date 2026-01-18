#pragma once

#include "physics/Category.h"

#include "loader/document.h"

namespace loader {
    struct PhysicsCategoryValue {
        physics::Category loadCategory(
            const loader::DocNode& value) const;

        void loadMask(
            const loader::DocNode& node,
            uint32_t& mask) const;
    };
}
