#pragma once

#include <vector>

#include "util/Ref.h"

#include "ki/size.h"

namespace pool
{
    struct NodeHandle;
}

namespace model
{
    class Node;
}

class NodeCache : public util::RefCounted<> {
public:
    NodeCache();
    ~NodeCache();

    void clear();

    void clearAt(size_t entityIndex);

    size_t size() const noexcept
    {
        return m_nodes.size();
    }

    void cacheNodes(
        const std::vector<pool::NodeHandle>& handles,
        ki::level_id nodeLevel);

    const std::vector<model::Node*>& getNodes() const noexcept
    {
        return m_nodes;
    }

private:
    // INDEX = entityIndex
    std::vector<model::Node*> m_nodes;
    ki::level_id m_level{ 0 };
};
