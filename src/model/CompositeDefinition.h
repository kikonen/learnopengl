#pragma once

#include "NodeDefinition.h"

#include "ki/size.h"

namespace model
{
    // For defining composite types
    struct CompositeDefinition
    {
        ki::composite_id m_id;

        std::shared_ptr<std::vector<NodeDefinition>> m_nodes;
    };
}
