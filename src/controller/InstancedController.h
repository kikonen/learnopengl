#pragma once

#include "NodeController.h"


class InstancedController : public NodeController
{
public:
    InstancedController();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) noexcept override;

protected:
    virtual void prepareInstanced(
        const Assets& assets,
        Registry* registry,
        Node& node);

    virtual bool updateInstanced(
        const RenderContext& ctx,
        Node& node,
        Node* parent);
};

