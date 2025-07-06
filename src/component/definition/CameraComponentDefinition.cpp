#include "CameraComponentDefinition.h"

#include "model/NodeType.h"

#include "component/CameraComponent.h"

#include "component/FpsCamera.h"
#include "component/FollowCamera.h"
#include "component/OrbitCamera.h"
#include "component/SplineCamera.h"

std::unique_ptr<CameraComponent> CameraComponentDefinition::createCameraComponent(
    const NodeType* type)
{
    if (!type->m_cameraComponentDefinition) return nullptr;

    const auto& data = *type->m_cameraComponentDefinition;

    // NOTE only node cameras in scenefile for now
    std::unique_ptr<CameraComponent> component;

    switch (data.m_type) {
    case CameraType::fps: {
        auto c = std::make_unique<FpsCamera>();
        c->setPitch(glm::radians(data.m_pitch));
        c->setPitchSpeed(glm::radians(data.m_pitchSpeed));
        component = std::move(c);
        break;
    }
    case CameraType::follow: {
        auto c = std::make_unique<FollowCamera>();
        c->m_springConstant = data.m_springConstant;
        c->m_distance = data.m_distance;
        component = std::move(c);
        break;
    }
    case CameraType::orbit: {
        auto c = std::make_unique<OrbitCamera>();
        c->m_offset = data.m_offset;
        c->m_up = data.m_up;
        c->m_pitchSpeed = glm::radians(data.m_pitchSpeed);
        c->m_yawSpeed = glm::radians(data.m_yawSpeed);
        component = std::move(c);
        break;
    }
    case CameraType::spline: {
        auto c = std::make_unique<SplineCamera>();
        c->m_path = Spline{ data.m_path };
        c->m_speed = data.m_speed;
        component = std::move(c);
        break;
    }
    }

    component->m_enabled = true;
    component->m_default = data.m_default;

    {
        auto& camera = component->getCamera();
        if (data.m_orthogonal) {
            camera.setViewport(data.m_viewport);
        }
        camera.setAxis(data.m_front, data.m_up);
        camera.setFov(data.m_fov);
    }

    return component;
}
