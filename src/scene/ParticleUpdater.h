#pragma once

#include <memory>

struct UpdateContext;
class Registry;

class ParticleUpdater
{
public:
    ParticleUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    ~ParticleUpdater();

    void destroy();

    bool isRunning() const;

    void prepare();

    void start();
    void run();

    void update(const UpdateContext& ctx);

private:
    bool m_loaded{ false };

    std::atomic<bool> m_running;
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<Registry> m_registry;
};
