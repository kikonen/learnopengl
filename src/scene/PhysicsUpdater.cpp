#include "PhysicsUpdater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"
#include "ki/FpsCounter.h"

#include "util/thread.h"
#include "util/Log.h"

#include "physics/PhysicsEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"


#define KI_TIMER(x)

namespace {
    size_t count = 0;
}

PhysicsUpdater::PhysicsUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_registry(registry),
    m_alive(alive)
{}

PhysicsUpdater::~PhysicsUpdater()
{
    KI_INFO("PHYSICS_UPDATER: destroy");
}

void PhysicsUpdater::destroy()
{
}

bool PhysicsUpdater::isRunning() const
{
    return m_running;
}

void PhysicsUpdater::prepare()
{
}

void PhysicsUpdater::start()
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

void PhysicsUpdater::run()
{
    ki::RenderClock clock;

    KI_INFO(fmt::format("PE: started - worker={}", util::isWorkerThread()));
    prepare();

    //const int delay = (int)(1000.f / 60.f);
    const int delay = 5;

    ki::FpsCounter fpsCounter;

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedDuration;

    auto& pe = physics::PhysicsEngine::get();

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
            KI_INFO(fmt::format("{} - objects={}, dynamic={}, static={}",
                fpsCounter.formatSummary("PE"),
                pe.getObjectCount(),
                pe.getDynamicBoundsCount(),
                pe.getStaticBoundsCount()));
        }

            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    KI_INFO(fmt::format("PE: stopped - worker={}", util::isWorkerThread()));
}

void PhysicsUpdater::update(const UpdateContext& ctx)
{
    auto& pe = physics::PhysicsEngine::get();

    if (!pe.isEnabled()) return;

    pe.updateBounds(ctx);
    pe.updateWT(ctx);
}
