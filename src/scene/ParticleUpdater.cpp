#include "ParticleUpdater.h"

#include <fmt/format.h>

#include "particle/ParticleSystem.h"

ParticleUpdater::ParticleUpdater(
    Engine& engine)
    : Updater("PS", 20, engine)
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
