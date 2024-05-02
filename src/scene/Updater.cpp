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

void Updater::prepare()
{
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
                KI_CRITICAL(ex.what());
            }
            catch (...) {
                m_running = false;
            }
        }
    };
    th.detach();
}

void Updater::run()
{
    ki::RenderClock clock;

    KI_INFO(fmt::format("AS: started - worker={}", util::isWorkerThread()));
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

        auto ts = duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );
        clock.ts = static_cast<float>(ts.count());
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

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    KI_INFO(fmt::format("{}: stopped - worker={}", m_prefix, util::isWorkerThread()));
}
