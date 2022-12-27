#pragma once

#include "asset/Assets.h"
#include "model/Node.h"

#include "gui/Input.h"

class NodeController
{
public:
    NodeController();

    virtual void prepare(const Assets& assets, Node& node) ;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) noexcept;

    virtual void onKey(Input* input, const ki::RenderClock& clock) {};
    virtual void onMouseMove(Input* input, double xoffset, double yoffset) {};
    virtual void onMouseScroll(Input* input, double xoffset, double yoffset) {};

protected:
    bool m_prepared = false;
};

