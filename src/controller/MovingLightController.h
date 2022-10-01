#pragma once

#include "NodeController.h"


class MovingLightController final : public NodeController
{
public:
    MovingLightController(
        const glm::vec3& center, 
        float radius, 
        float speed);

    bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    const glm::vec3 m_center;
    const float m_radius;
    const float m_speed;
};


