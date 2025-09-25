#pragma once

#include "pool/NodeHandle.h"
#include "CreateState.h"

#include "ki/size.h"

namespace model
{
    struct ResolvedNode {
        // NOTE KI SID of parent known, not node yet
        ki::node_id parentId;
        ki::socket_id socketId;
        pool::NodeHandle handle;
        bool active;

        model::CreateState state;
    };
}
