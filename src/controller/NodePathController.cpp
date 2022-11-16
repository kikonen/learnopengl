#include "NodePathController.h"

NodePathController::NodePathController(
    const glm::vec3& center,
    int pathMode)
    : center(center),
    pathMode(pathMode)
{
    m_translate = pathMode == 1;
    m_rotate = pathMode == 1 || pathMode == 3;
    m_scale = pathMode == 1 || pathMode == 4;
}

bool NodePathController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    float elapsed = ctx.m_clock.ts;

    if (m_translate) {
        const float radius = 8.0f;
        const float posX = sin(elapsed / 0.9f) * radius;
        const float posY = sin(elapsed * 1.1f) * radius / 3.0f + 15.f;
        const float posZ = cos(elapsed) * radius / 2.0f;

        auto pos = center + glm::vec3(posX, posY, posZ);
        //if (!parent) pos += ctx.assets.groundOffset,

        node.setPosition(pos);
    }

    if (m_rotate) {
        const float radius = 2.0f;
        const float rotX = elapsed * radius;
        const float rotY = elapsed * radius * 1.1f;
        const float rotZ = elapsed * radius * 1.2f;

        node.setRotation(glm::vec3(rotX, rotY, rotZ));
    }

    if (m_scale) {
        const float radius = 0.5f;
        float scale = 1.1f + sin(elapsed / 4.0f) * radius;
        assert(scale > 0);

        node.setScale(scale);
    }

    return true;
}
