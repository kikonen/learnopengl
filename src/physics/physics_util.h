#pragma once

#include "util/util.h"

#include "Category.h"

namespace physics {
    template <typename... Categories>
    uint32_t mask(Categories... values)
    {
        uint32_t mask = 0;
        for (const auto c : { values... }) {
            mask |= util::as_integer(c);
        }
        return mask;
    }
}
