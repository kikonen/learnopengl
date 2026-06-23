#include "ClockController.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "generator/TextGenerator.h"

#include "scene/Scene.h"
#include "scene/World.h"

ClockController::ClockController(std::string_view fmtSpec)
    : NodeController(false, false),
    m_format{ fmtSpec.empty() ? "%Y-%m-%d %H:%M" : std::string{ fmtSpec } }
{
}

bool ClockController::updateWT(
    const UpdateContext& ctx,
    model::Node& node) noexcept
{
    auto* scene = ctx.getScene();
    if (!scene) return false;

    auto* world = scene->getWorld().get();
    if (!world) return false;

    auto* generator = node.getGenerator<TextGenerator>();
    if (!generator) return false;

    // WT-authoritative time (advanced this tick before controllers run)
    const double t = world->getTime().getTimeSecs();
    std::string text = World::formatClock(t, m_format);

    if (text != m_last) {
        generator->setText(text);
        m_last = std::move(text);
    }

    // does not move the node
    return false;
}
