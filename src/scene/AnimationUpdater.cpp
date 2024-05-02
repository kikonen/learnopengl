#include "AnimationUpdater.h"

#include "util/Log.h"

#include "animation/Animator.h"

#include "registry/Registry.h"
#include "registry/BoneRegistry.h"


#define KI_TIMER(x)

namespace {
}

AnimationUpdater::AnimationUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("AS", 20, registry, alive)
{}

uint32_t AnimationUpdater::getActiveCount() const noexcept
{
    return BoneRegistry::get().getActiveBoneCount();
}

void AnimationUpdater::update(const UpdateContext& ctx)
{
    BoneRegistry::get().updateWT(ctx);
}
