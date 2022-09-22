#include "CameraController.h"


CameraController::CameraController()
{
}

bool CameraController::update(const RenderContext& ctx, Node& node)
{
    glm::vec3 viewFront = node.camera->getViewFront();
    glm::vec3 viewUp = node.camera->getViewUp();
    glm::vec3 pos = node.camera->getPos();// +(front * 0.1f);
    glm::vec3 rot = node.camera->getRotation();

    auto nodePos = pos - viewUp * 2.8f + viewFront * 9.f;

    node.setPos(nodePos);
    node.setRotation({-rot.x, 90 + rot.y, rot.z});

    return false;
}
