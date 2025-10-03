#pragma once

#include <string>
#include <memory>
#include <mutex>

#include "event/Listen.h"

struct UpdateContext;
class Engine;
class Registry;

class Updater
{
public:
    Updater(
        std::string_view prefix,
        size_t delay,
        Engine& engine,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual ~Updater();

    void destroy();

    bool isRunning() const;

    virtual void shutdown();

    virtual void prepare();

    bool isPrepared() const noexcept {
        std::lock_guard lock(m_prepareLock);
        return m_prepared;
    }

    void start();
    void run();

    virtual void update(const UpdateContext& ctx) = 0;

    virtual std::string getStats() = 0;

    Registry* getRegistry() const noexcept;

protected:
    bool m_loaded{ false };

    const std::string m_prefix;
    const size_t m_delay;

    mutable std::mutex m_prepareLock;
    bool m_prepared{ false };

    std::atomic<bool> m_running;
    std::shared_ptr<std::atomic<bool>> m_alive;

    Engine& m_engine;
};
