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
    Engine& engine)
    : Updater("AS", 22, engine)
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
