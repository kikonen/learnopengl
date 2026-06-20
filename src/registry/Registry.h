#pragma once

#include <functional>
#include <atomic>
#include <mutex>

#include "util/Ref.h"

#include "event/Task.h"

namespace event
{
    class Dispatcher;
}

namespace render
{
    class InstanceRegistry;
}

struct UpdateContext;

class Engine;
class NodeRegistry;
class SelectionRegistry;

struct PrepareContext;

//
// Container for all registries to simplify passing them around
//
class Registry : public util::RefCounted<> {
public:
    Registry(
        const util::Ref<Engine>& engine,
        const std::shared_ptr<std::atomic_bool>& alive);

    ~Registry();

    void clear();
    void prepare(const PrepareContext& ctx);

    void updateWT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);

    void startFrame();
    void endFrame();

    void withLock(const std::function<void(Registry&)>& fn);

    // Defer work to the next worker-thread (WT) dispatcher drain.
    void invokeLaterWT(event::Task task);

    // Defer work to the next view/render-thread (RT) dispatcher drain.
    void invokeLaterRT(event::Task task);

    const util::Ref<Engine>& getEngine() noexcept
    {
        return m_engine;
    }

private:
    bool m_prepared = false;

    util::Ref<Engine> m_engine;
    std::shared_ptr<std::atomic_bool> m_alive;
    std::mutex m_lock{};

    std::unique_ptr<event::Dispatcher> m_dispatcherWorkerImpl;
    std::unique_ptr<event::Dispatcher> m_dispatcherViewImpl;

public:
    // NOTE KI initialization order!
    event::Dispatcher* const m_dispatcherWorker;
    event::Dispatcher* const m_dispatcherView;

    NodeRegistry* const m_nodeRegistry;
    render::InstanceRegistry* const m_instanceRegistry;
    SelectionRegistry* const m_selectionRegistry;
};
