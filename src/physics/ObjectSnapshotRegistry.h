#pragma once

#include "physics/ObjectSnapshot.h"

#include "registry/SnapshotRegistry.h"

namespace physics {
    class ObjectSnapshotRegistry final : public SnapshotRegistry<ObjectSnapshot> {
    public:
        ObjectSnapshotRegistry();
        ~ObjectSnapshotRegistry();
    };
}
