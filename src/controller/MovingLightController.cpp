#include "MovingLightController.h"

#include "component/Light.h"

MovingLightController::MovingLightController(
    const Assets& assets, 
    const glm::vec3& center, 
    float radius,
    float speed,
    Node* node)
    : NodeController(assets),
    center(center),
    radius(radius),
    speed(speed),
    node(node)
{
}

bool MovingLightController::update(const RenderContext& ctx, Node& node)
{
    Light* light = node.light;
    if (!light) return false;

    float elapsed = ctx.clock.ts / speed;

    float posX = sin(elapsed) * radius;
    float posY = sin(elapsed / 2) * 2;
    float posZ = cos(elapsed) * radius;

    glm::vec3 pos = glm::vec3(posX, posY, posZ) + center;

    light->setPos(pos);
    light->update(ctx);
    node.setPos(pos);
//    node.setScale(light->radius);

    return true;
}


