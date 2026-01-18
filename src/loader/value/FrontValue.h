#pragma once

#include "util/Axis.h"

#include "loader/document.h"
#include "loader/loader_util.h"

namespace loader {
    struct FrontValue {
        static util::Front load(const loader::DocNode& node)
        {
            std::string str = readString(node);
            if (str == "z") return util::Front::z;
            if (str == "x") return util::Front::x;
            if (str == "-x") return util::Front::neg_x;
            if (str == "-z") return util::Front::neg_z;
            return util::Front::z;
        }
    };
}
