#pragma once

#include <functional>
#include <atomic>
#include <mutex>

namespace event
{
    class Dispatcher;
}

struct UpdateContext;

class NodeRegistry;

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
    void prepareRT();

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

public:
    // NOTE KI initialization order!
    event::Dispatcher* const m_dispatcherWorker;
    event::Dispatcher* const m_dispatcherView;

    NodeRegistry* const m_nodeRegistry;
};
