#include "MovingLightController.h"

#include "component/Light.h"

#include "scene/RenderContext.h"

MovingLightController::MovingLightController(
    const glm::vec3& center,
    float radius,
    float speed)
  : m_center(center),
    m_radius(radius),
    m_speed(speed)
{
}

bool MovingLightController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    assert(m_speed > 0);
    const float elapsed = ctx.m_clock.ts / m_speed;

    const float posX = sin(elapsed) * m_radius;
    const float posY = sin(elapsed / 2) * 2;
    const float posZ = cos(elapsed) * m_radius;

    const glm::vec3 pos = m_center + glm::vec3(posX, posY, posZ);

    node.setPosition(pos);

    return true;
}
