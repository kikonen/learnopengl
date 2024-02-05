#pragma once

#include <functional>
#include <atomic>
#include <mutex>

namespace event
{
    class Dispatcher;
}

struct UpdateContext;

class SnapshotRegistry;

//
// Container for all registries to simplify passing them around
//
class Registry {
public:
    static Registry& get() noexcept;

    Registry();

    ~Registry();

    void prepareShared(std::shared_ptr<std::atomic<bool>> alive);
    void prepareWT();

    void updateWT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);

    void withLock(const std::function<void(Registry&)>& fn);

private:
    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;
    std::mutex m_lock{};

    std::unique_ptr<event::Dispatcher> m_dispatcherImpl;
    std::unique_ptr<event::Dispatcher> m_dispatcherViewImpl;

    std::unique_ptr<SnapshotRegistry> m_snapshotRegistryImpl;

public:
    // NOTE KI initialization order!
    event::Dispatcher* const m_dispatcher;
    event::Dispatcher* const m_dispatcherView;

    SnapshotRegistry* const m_snapshotRegistry;
};
