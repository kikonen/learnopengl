#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "EntityCloneData.h"

namespace loader {
    struct ResolvedEntity {
        ki::node_id parentId;
        pool::NodeHandle handle;
        const EntityCloneData& data;
    };
}
