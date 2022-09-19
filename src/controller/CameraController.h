#pragma once

#include "NodeController.h"

class CameraController : public NodeController
{
public:
    CameraController(const std::shared_ptr<Assets> assets);
    ~CameraController();

    bool update(const RenderContext& ctx, Node& node) override;
};
