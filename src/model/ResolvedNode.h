#pragma once

#include "pool/NodeHandle.h"
#include "CreateState.h"

#include "ki/size.h"

struct ResolvedNode {
    ki::node_id parentId;
    pool::NodeHandle handle;
    bool active;

    CreateState state;
};
