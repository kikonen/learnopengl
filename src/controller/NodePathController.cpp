#include "NodePathController.h"

NodePathController::NodePathController(
    int pathMode)
    : pathMode(pathMode)
{
}

bool NodePathController::update(const RenderContext& ctx, Node& node, Node* parent)
{
    float elapsed = ctx.clock.ts;

    if (true) {
        const float radius = 4.0f;
        float posX = sin(elapsed / 0.9f) * radius;
        float posY = sin(elapsed * 1.1f) * radius / 3.0f + 15.f;
        float posZ = cos(elapsed) * radius / 2.0f;

        auto pos = glm::vec3(posX, posY, posZ);
        if (!parent) pos += ctx.assets.groundOffset;

        node.setPos(pos);
    }

    if (true) {
        const float radius = 2.0f;
        float rotX = elapsed * radius;
        float rotY = elapsed * radius * 1.1f;
        float rotZ = elapsed * radius * 1.2f;

        node.setRotation(glm::vec3(rotX, rotY, rotZ));
    }

    if (true) {
        const float radius = 2.0f;
        float scale = sin(elapsed / 4.0f) * radius;

        node.setScale(scale);
    }

    return true;
}
