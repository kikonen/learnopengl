#pragma once

#include "NodeController.h"

class NodePathController : public NodeController
{
public:
    NodePathController(const std::shared_ptr<Assets> assets, int pathMode);
    ~NodePathController();

    bool update(const RenderContext& ctx, Node& node) override;

private:
    const int pathMode;

};

