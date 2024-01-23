#pragma once

#include <vector>

#include "ResolvedEntity.h"

namespace loader {
    struct DagSort {
        std::vector<ResolvedEntity*> sort(std::vector<ResolvedEntity>& resolvedEntities);
    };
}
