#include "ParticleUpdater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"
#include "ki/FpsCounter.h"

#include "util/thread.h"
#include "util/Log.h"

#include "particle/ParticleSystem.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"


#define KI_TIMER(x)

namespace {
    size_t count = 0;
}

ParticleUpdater::ParticleUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_registry(registry),
    m_alive(alive)
{}

ParticleUpdater::~ParticleUpdater()
{
    KI_INFO("PARTICLE_UPDATER: destroy");
}

void ParticleUpdater::destroy()
{
}

bool ParticleUpdater::isRunning() const
{
    return m_running;
}

void ParticleUpdater::prepare()
{
}

void ParticleUpdater::start()
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

void ParticleUpdater::run()
{
    ki::RenderClock clock;

    std::cout << "PT: worker=" << util::isWorkerThread() << '\n';
    prepare();

    //const int delay = (int)(1000.f / 60.f);
    const int delay = 5;

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
            KI_INFO(fmt::format("{} - particles: {}", fpsCounter.formatSummary("PT"), particle::ParticleSystem::get().getActiveParticleCount()));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void ParticleUpdater::update(const UpdateContext& ctx)
{
    particle::ParticleSystem::get().updateWT(ctx);
}
