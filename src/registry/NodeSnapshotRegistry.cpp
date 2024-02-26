#include "NodeSnapshotRegistry.h"

#include "SnapshotRegistry_impl.h"

NodeSnapshotRegistry::NodeSnapshotRegistry()
    : SnapshotRegistry{}
{
}

NodeSnapshotRegistry::~NodeSnapshotRegistry() = default;

void NodeSnapshotRegistry::cacheNodes(std::vector<Node*>& cache) const noexcept
{
    const auto& entries = m_snapshots->m_entries;

    cache.resize(entries.size());

    for (size_t i = 0; i < entries.size(); i++) {
        cache[i] = entries[i].m_handle.toNode();
    }
}
