#pragma once

#include "NodeController.h"

class NodePathController final : public NodeController
{
public:
    NodePathController(int pathMode);

    bool update(const RenderContext& ctx, Node& node) override;

private:
    const int pathMode;

};

