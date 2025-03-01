#include "Updater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"
#include "ki/FpsCounter.h"

#include "util/thread.h"
#include "util/Log.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"


#define KI_TIMER(x)

namespace {
    size_t count = 0;
}

Updater::Updater(
    std::string_view prefix,
    size_t delay,
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_prefix{ prefix },
    m_delay{ delay },
    m_registry(registry),
    m_alive(alive)
{}

Updater::~Updater()
{
    KI_INFO(fmt::format("{}_UPDATER: destroy", m_prefix));
}

void Updater::destroy()
{
}

bool Updater::isRunning() const
{
    return m_running;
}

void Updater::shutdown()
{
}

void Updater::prepare()
{
    std::lock_guard lock(m_prepareLock);
    m_prepared = true;
}

void Updater::start()
{
    auto th = std::thread{
        [this]() mutable {
            try {
                m_running = true;
                util::markWorkerThread();
                run();
                m_running = false;
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(fmt::format("UPDATER_ERROR: {}", ex.what()));
                m_running = false;
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("UPDATER_ERROR: {}", ex));
                m_running = false;
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("UPDATER_ERROR: {}", ex));
            }
            catch (...) {
                KI_CRITICAL("UPDATER_ERROR: UNKNOWN_ERROR");
                m_running = false;
            }
        }
    };
    th.detach();
}

void Updater::run()
{
    ki::RenderClock clock;

    KI_INFO(fmt::format("{}: started - worker={}", m_prefix, util::isWorkerThread()));
    prepare();

    //const int delay = (int)(1000.f / 60.f);
    const size_t delay = m_delay;

    ki::FpsCounter fpsCounter;

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedDuration;

    while (*m_alive) {
        fpsCounter.startFame();

        loopTime = std::chrono::system_clock::now();
        elapsedDuration = loopTime - prevLoopTime;
        prevLoopTime = loopTime;

        auto ts = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );
        clock.ts = static_cast<double>(ts.count()) / (1000.0 * 1000.0);
        clock.elapsedSecs = elapsedDuration.count();

        UpdateContext ctx(
            clock,
            m_registry.get());

        update(ctx);

        fpsCounter.endFame(clock.elapsedSecs);

        if (fpsCounter.isUpdate())
        {
            KI_INFO(fmt::format("{} - active={}",
                fpsCounter.formatSummary(m_prefix),
                getActiveCount()));
        }

        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    shutdown();
    KI_INFO(fmt::format("{}: stopped - worker={}", m_prefix, util::isWorkerThread()));
}
