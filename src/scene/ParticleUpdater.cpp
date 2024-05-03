#include "ParticleUpdater.h"

#include "particle/ParticleSystem.h"

ParticleUpdater::ParticleUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("PS", 20, registry, alive)
{}

uint32_t ParticleUpdater::getActiveCount() const noexcept
{
    return particle::ParticleSystem::get().getActiveParticleCount();
}

void ParticleUpdater::update(const UpdateContext& ctx)
{
    particle::ParticleSystem::get().updateWT(ctx);
}
