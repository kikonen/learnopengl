#pragma once

#include <vector>

#include "NodeData.h"

namespace loader {
    struct NodeRoot {
        NodeData base;
        std::vector<NodeData> clones;
    };
}
