#include "NodeCache.h"

#include "pool/NodeHandle.h"

namespace
{
    constexpr int INITIAL_SIZE = 30000;
}

NodeCache::NodeCache()
{ }

NodeCache::~NodeCache() = default;

void NodeCache::clear()
{
    m_nodes.clear();
    m_level = 0;
    m_nodes.reserve(INITIAL_SIZE);
}

void NodeCache::clearAt(size_t entityIndex)
{
    if (entityIndex < m_nodes.size()) {
        m_nodes[entityIndex] = nullptr;
    }
}

void NodeCache::cacheNodes(
    const std::vector<pool::NodeHandle>& handles,
    ki::level_id nodeLevel)
{
    if (m_level == nodeLevel) return;
    m_level = nodeLevel;

    auto& cache = m_nodes;

    cache.resize(handles.size());
    cache[ki::NULL_ENTITY_INDEX] = nullptr;
    cache[ki::ID_ENTITY_INDEX] = nullptr;

    for (size_t i = ki::ID_ENTITY_INDEX + 1; i < handles.size(); i++) {
        if (!handles[i]) continue;
        cache[i] = handles[i].toNode();
    }
}
