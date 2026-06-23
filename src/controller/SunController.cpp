#include "SunController.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"
#include "model/NodeState.h"

#include "registry/NodeRegistry.h"

#include "scene/Scene.h"
#include "scene/World.h"

SunController::SunController(float distance)
    : NodeController(false, false),
    m_distance{ distance }
{
}

bool SunController::updateWT(
    const UpdateContext& ctx,
    model::Node& node) noexcept
{
    auto* scene = ctx.getScene();
    if (!scene) return false;

    auto* world = scene->getWorld().get();
    if (!world) return false;

    // WT-authoritative time (already advanced this tick by SceneUpdater before controllers)
    const double t = world->getTime().getTimeSecs();
    const glm::vec3 pos = world->sunDirectionToSun(t) * m_distance;

    auto& state = NodeRegistry::get().modifyState(node.getEntityIndex());
    state.setPosition(pos);
    return true;
}
