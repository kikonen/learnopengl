#include "CameraController.h"


CameraController::CameraController(const Assets& assets)
    : NodeController(assets)
{
}

CameraController::~CameraController()
{
}

bool CameraController::update(const RenderContext& ctx, Node& node)
{
    glm::vec3 front = node.camera->getFront();
    glm::vec3 pos = node.camera->getPos();// +(front * 0.1f);
    glm::vec3 rot = node.camera->getRotation();

    node.setPos({pos.x, pos.y, pos.z });
    node.setRotation({-rot.x, 180 + rot.y, rot.z});

    return false;
}
