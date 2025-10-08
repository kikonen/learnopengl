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
        Engine& engine);

    virtual ~Updater();

    void destroy();

    bool isRunning() const;

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
    virtual void shutdown();

protected:
    bool m_loaded{ false };

    const std::string m_prefix;
    const size_t m_delay;

    mutable std::mutex m_prepareLock;
    bool m_prepared{ false };

    std::atomic_bool m_alive;
    std::atomic_bool m_running;

    Engine& m_engine;
};
