#pragma once

#include <vector>

#include "ResolvedNode.h"

namespace loader {
    struct DagSort {
        std::vector<ResolvedNode*> sort(std::vector<ResolvedNode>& resolvedEntities);
    };
}
