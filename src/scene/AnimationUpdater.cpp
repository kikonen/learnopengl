#include "AnimationUpdater.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "animation/Animator.h"

#include "registry/Registry.h"

#include "animation/AnimationSystem.h"


#define KI_TIMER(x)

namespace {
}

AnimationUpdater::AnimationUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("AS", 22, registry, alive)
{}

void AnimationUpdater::update(const UpdateContext& ctx)
{
    animation::AnimationSystem::get().updateWT(ctx);
}

std::string AnimationUpdater::getStats()
{
    const auto animCount = animation::AnimationSystem::get().getActiveCount();
    return fmt::format("animations={}", animCount);
}
