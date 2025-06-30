#pragma once

#include <vector>

struct ResolvedNode;

struct DagSort {
    std::vector<ResolvedNode*> sort(
        std::vector<ResolvedNode>& resolvedEntities);
};
