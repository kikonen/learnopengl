#pragma once

#include "NodeController.h"

class InstancedNode;

class InstancedController : public NodeController
{
public:
    InstancedController(const Assets& assets);
    ~InstancedController();

    virtual void prepare(Node& node) override;
    virtual bool update(const RenderContext& ctx, Node& node) override;

    virtual void prepareInstanced(InstancedNode& node);
    virtual bool updateInstanced(const RenderContext& ctx, InstancedNode& node);

};

