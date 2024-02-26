#pragma once

#include "pool/NodeHandle.h"

namespace physics {
    struct NodeBounds {
        uint32_t m_objectSnapshotIndex{ 0 };
        uint32_t m_nodeSnapshotIndex{ 0 };

        pool::NodeHandle m_nodeHandle{};

        ki::level_id m_matrixLevel{ 0 };

        bool m_static : 1 { false };
    };
}
