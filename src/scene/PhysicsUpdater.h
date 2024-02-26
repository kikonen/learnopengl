#pragma once

#include <memory>

struct UpdateContext;
class Registry;
class Node;

class PhysicsUpdater
{
public:
    PhysicsUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    ~PhysicsUpdater();

    void destroy();

    bool isRunning() const;

    void prepare();

    void start();
    void run();

    void update(const UpdateContext& ctx);

private:
    std::atomic<bool> m_running;
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<Registry> m_registry;
};
