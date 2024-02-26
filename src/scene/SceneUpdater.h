#pragma once

#include <memory>
#include <mutex>

struct UpdateContext;
class Registry;
class Node;

class SceneUpdater
{
public:
    SceneUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    ~SceneUpdater();

    void destroy();

    bool isRunning() const;

    void prepare();

    bool isPrepared() const noexcept {
        std::lock_guard lock(m_prepareLock);
        return m_prepared;
    }

    void start();
    void run();

    void update(const UpdateContext& ctx);

private:
    void handleNodeAdded(Node* node);

private:
    bool m_loaded{ false };
    bool m_prepared{ false };

    mutable std::mutex m_prepareLock;

    std::atomic<bool> m_running;
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<Registry> m_registry;
};
