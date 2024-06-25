#include "AnimationUpdater.h"

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

uint32_t AnimationUpdater::getActiveCount() const noexcept
{
    return animation::AnimationSystem::get().getActiveCount();
}

void AnimationUpdater::update(const UpdateContext& ctx)
{
    animation::AnimationSystem::get().updateWT(ctx);
}
