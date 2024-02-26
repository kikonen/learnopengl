#include "ObjectSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"

namespace physics {
    ObjectSnapshotRegistry::ObjectSnapshotRegistry()
        : SnapshotRegistry{}
    {
    }

    ObjectSnapshotRegistry::~ObjectSnapshotRegistry() = default;
}
