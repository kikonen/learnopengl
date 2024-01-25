#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "EntityData.h"

namespace loader {
    struct ResolvedEntity {
        ki::node_id parentId;
        pool::NodeHandle handle;
        const EntityCloneData& data;
    };
}
