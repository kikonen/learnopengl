#pragma once

#include "asset/Assets.h"
#include "model/Node.h"

class NodeController
{
public:
    NodeController();

    virtual void prepare(const Assets& assets, Node& node);

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent);

protected:
    bool m_prepared = false;
};

