#pragma once

#include "NodeController.h"

class CameraController final : public NodeController
{
public:
    CameraController();

    bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) noexcept override;
};
