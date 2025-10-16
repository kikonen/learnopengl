#include "Updater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"
#include "ki/FpsCounter.h"

#include "util/Util.h"
#include "util/thread.h"
#include "util/Log.h"

#include "engine/Engine.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"


#define KI_TIMER(x)

namespace {
    size_t count = 0;
}

Updater::Updater(
    std::string_view prefix,
    size_t delay,
    Engine& engine)
    : m_prefix{ prefix },
    m_delay{ delay },
    m_engine{ engine }
{}

Updater::~Updater()
{
    KI_INFO(fmt::format("{}_UPDATER: deleted", m_prefix));
}

void Updater::destroy()
{
    m_alive = false;
    KI_INFO(fmt::format("{}_UPDATER: shutdownRequested", m_prefix));
}

bool Updater::isRunning() const
{
    return m_running;
}

void Updater::shutdown()
{
    KI_INFO(fmt::format("{}_UPDATER: shutdown", m_prefix));
}

void Updater::prepare()
{
    std::lock_guard lock(m_prepareLock);
    m_prepared = true;
}

void Updater::start()
{
    m_alive = true;
    auto th = std::thread{
        [this]() mutable {
            try {
                m_running = true;
                util::markWorkerThread();
                run();
                shutdown();
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

    auto prevLoopTime = std::chrono::high_resolution_clock::now();
    auto loopTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedDuration;

    while (m_alive) {
        fpsCounter.startFrame();
        fpsCounter.startRender();

        loopTime = std::chrono::high_resolution_clock::now();
        elapsedDuration = loopTime - prevLoopTime;
        prevLoopTime = loopTime;

        auto ts = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        );
        clock.ts = static_cast<double>(ts.count()) / (1000.0 * 1000.0);
        clock.elapsedSecs = elapsedDuration.count();

        UpdateContext ctx{ m_engine, clock };

        update(ctx);

        fpsCounter.endRender();
        fpsCounter.endFrame();

        if (fpsCounter.isUpdate())
        {
            KI_INFO(fmt::format("{} - {}",
                fpsCounter.formatSummary(m_prefix),
                getStats()));
        }

        if (delay > 0) {
            util::sleep(delay);
        }
    }

    KI_INFO(fmt::format("{}: stopped - worker={}", m_prefix, util::isWorkerThread()));
}

Registry* Updater::getRegistry() const noexcept
{
    return m_engine.getRegistry();
}
