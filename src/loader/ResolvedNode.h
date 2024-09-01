#pragma once

#include <vector>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "script/size.h"

#include "model/NodeState.h"

#include "NodeData.h"

namespace loader {
    struct ResolvedNode {
        ki::node_id parentId;
        pool::NodeHandle handle;
        const NodeData& data;

        NodeState state;

        const std::vector<script::script_id> scriptIds;
    };
}
