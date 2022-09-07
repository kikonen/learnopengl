#pragma once

#include "NodeController.h"

class CameraController : public NodeController
{
public:
    CameraController(const Assets& assets);
    ~CameraController();

    bool update(const RenderContext& ctx, Node& node) override;
};
