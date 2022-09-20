#pragma once

#include "NodeController.h"

class InstancedNode;

class InstancedController : public NodeController
{
public:
    InstancedController();

    virtual void prepare(const Assets& assets, Node& node) override;
    virtual bool update(const RenderContext& ctx, Node& node) override;

    virtual void prepareInstanced(const Assets& assets, InstancedNode& node);
    virtual bool updateInstanced(const RenderContext& ctx, InstancedNode& node);

};

