#pragma once

#include <functional>
#include <atomic>
#include <mutex>

namespace event
{
    class Dispatcher;
}

namespace physics {
    class ObjectSnapshotRegistry;
}

struct UpdateContext;

class NodeRegistry;
class NodeSnapshotRegistry;

//
// Container for all registries to simplify passing them around
//
class Registry {
public:
    Registry(
        std::shared_ptr<std::atomic<bool>> alive);

    ~Registry();

    void prepareShared();
    void prepareWT();

    void updateWT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);

    void withLock(const std::function<void(Registry&)>& fn);

private:
    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;
    std::mutex m_lock{};

    std::unique_ptr<event::Dispatcher> m_dispatcherWorkerImpl;
    std::unique_ptr<event::Dispatcher> m_dispatcherViewImpl;

    std::unique_ptr<NodeSnapshotRegistry> m_workerSnapshotRegistryImpl;
    std::unique_ptr<NodeSnapshotRegistry> m_pendingSnapshotRegistryImpl;
    std::unique_ptr<NodeSnapshotRegistry> m_activeSnapshotRegistryImpl;

    std::unique_ptr<physics::ObjectSnapshotRegistry> m_workerObjectSnapshotRegistryImpl;
    std::unique_ptr<physics::ObjectSnapshotRegistry> m_pendingObjectSnapshotRegistryImpl;
    std::unique_ptr<physics::ObjectSnapshotRegistry> m_activeObjectSnapshotRegistryImpl;

public:
    // NOTE KI initialization order!
    event::Dispatcher* const m_dispatcherWorker;
    event::Dispatcher* const m_dispatcherView;

    NodeSnapshotRegistry* const m_workerSnapshotRegistry;
    NodeSnapshotRegistry* const m_pendingSnapshotRegistry;
    NodeSnapshotRegistry* const m_activeSnapshotRegistry;

    physics::ObjectSnapshotRegistry* const m_workerObjectSnapshotRegistry;
    physics::ObjectSnapshotRegistry* const m_pendingObjectSnapshotRegistry;
    physics::ObjectSnapshotRegistry* const m_activeObjectSnapshotRegistry;

    NodeRegistry* const m_nodeRegistry;
};
