#pragma once

#include "NodeController.h"


class MovingLightController : public NodeController
{
public:
    MovingLightController(
        const std::shared_ptr<Assets> assets,
        const glm::vec3& center, 
        float radius, 
        float speed,
        Node* node);

    bool update(const RenderContext& ctx, Node& node) override;

private:
    const glm::vec3 center;
    const float radius;
    const float speed;

    Node* node;
};


