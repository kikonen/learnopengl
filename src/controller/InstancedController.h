#pragma once

#include "NodeController.h"

class InstancedNode;

class InstancedController : public NodeController
{
public:
    InstancedController();

    virtual void prepare(
        const Assets& assets,
        EntityRegistry& entityRegistry,
        Node& node) override;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) noexcept override;

protected:
    virtual void prepareInstanced(
        const Assets& assets,
        EntityRegistry& entityRegistry,
        InstancedNode& node);

    virtual bool updateInstanced(
        const RenderContext& ctx,
        InstancedNode& node,
        Node* parent);
};

