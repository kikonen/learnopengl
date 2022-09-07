#pragma once

#include "asset/Assets.h"
#include "model/Node.h"

class NodeController
{
public:
    NodeController(const Assets& assets);
    ~NodeController();

    virtual void prepare(Node& node);
    virtual bool update(const RenderContext& ctx, Node& node);

protected:
    const Assets& assets;
};

