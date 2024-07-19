#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "NodeData.h"

namespace loader {
    struct ResolvedNode {
        ki::node_id parentId;
        pool::NodeHandle handle;
        const NodeData& data;
    };
}
