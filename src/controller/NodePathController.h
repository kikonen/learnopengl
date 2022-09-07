#pragma once

#include "NodeController.h"

class NodePathController : public NodeController
{
public:
    NodePathController(const Assets& assets, int pathMode);
    ~NodePathController();

    bool update(const RenderContext& ctx, Node& node) override;

private:
    const int pathMode;

};

