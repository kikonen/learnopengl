#pragma once

#include <stdint.h>

enum class PivotAlignment : uint8_t {
    zero,
    middle,
    top,
    bottom,
    left,
    right
};

struct PivotPoint {
    PivotAlignment axis[3]{
        PivotAlignment::zero,
        PivotAlignment::zero,
        PivotAlignment::zero
    };
};

