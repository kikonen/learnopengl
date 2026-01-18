#pragma once

#include "util/Axis.h"

#include "loader/document.h"
#include "loader/loader_util.h"

namespace loader {
    struct AxisValue {
        static util::Axis load(const loader::DocNode& node)
        {
            std::string str = readString(node);
            if (str == "y") return util::Axis::y;
            if (str == "z") return util::Axis::z;
            if (str == "x") return util::Axis::x;
            if (str == "-y") return util::Axis::neg_y;
            if (str == "-z") return util::Axis::neg_z;
            if (str == "-x") return util::Axis::neg_x;
            return util::Axis::y;
        }
    };
}
