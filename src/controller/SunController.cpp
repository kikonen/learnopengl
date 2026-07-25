#include "SunController.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"
#include "model/NodeState.h"

#include "registry/NodeRegistry.h"

#include "scene/Scene.h"
#include "scene/World.h"

SunController::SunController(float distance, bool moon)
    : NodeController(false, false),
    m_distance{ distance },
    m_moon{ moon }
{
}

bool SunController::updateWT(
    const UpdateContext& ctx,
    model::Node& node) noexcept
{
    const auto& scene = ctx.getScene();
    if (!scene) return false;

    auto* world = scene->getWorld().get();
    if (!world) return false;

    // WT-authoritative time (already advanced this tick by SceneUpdater before controllers)
    const double t = world->getTime().getTimeSecs();
    glm::vec3 toward = world->sunDirectionToSun(t);
    if (m_moon) toward = -toward; // moon = anti-sun
    const glm::vec3 pos = toward * m_distance;

    auto& state = NodeRegistry::get().modifyState(node.getEntityIndex());
    state.setPosition(pos);
    return true;
}
