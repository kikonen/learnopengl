#pragma once

#include "model/Snapshot.h"

#include "registry/SnapshotRegistry.h"

class Node;

class NodeSnapshotRegistry final : public SnapshotRegistry<Snapshot> {
public:
    NodeSnapshotRegistry();
    ~NodeSnapshotRegistry();

    void cacheNodes(std::vector<Node*>& cache) const noexcept;
};
