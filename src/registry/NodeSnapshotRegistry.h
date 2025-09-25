#pragma once

#include "model/Snapshot.h"

#include "registry/SnapshotRegistry.h"

class NodeSnapshotRegistry final : public SnapshotRegistry<model::Snapshot> {
public:
    NodeSnapshotRegistry();
    ~NodeSnapshotRegistry();
};
