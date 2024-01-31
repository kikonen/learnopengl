#pragma once

#include <memory>

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

    void start();
    void run();

    void update(const UpdateContext& ctx);

private:
    void handleNodeAdded(Node* node);

private:
    bool m_loaded{ false };

    std::atomic<bool> m_running;
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<Registry> m_registry;
};
