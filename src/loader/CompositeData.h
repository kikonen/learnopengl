#pragma once

#include <vector>
#include <memory>

#include "BaseId.h"

namespace loader {
    struct NodeData;

    struct CompositeData {
        BaseId baseId;

        std::shared_ptr<std::vector<NodeData>> nodes;
    };
}
