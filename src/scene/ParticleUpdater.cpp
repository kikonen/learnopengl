#include "ParticleUpdater.h"

#include <fmt/format.h>

#include "particle/ParticleSystem.h"

ParticleUpdater::ParticleUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("PS", 20, registry, alive)
{}

void ParticleUpdater::update(const UpdateContext& ctx)
{
    particle::ParticleSystem::get().updateWT(ctx);
}

std::string ParticleUpdater::getStats()
{
    const auto particleCount = particle::ParticleSystem::get().getActiveParticleCount();
    return fmt::format("particles={}", particleCount);
}
