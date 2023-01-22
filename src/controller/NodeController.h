#pragma once

#include "asset/Assets.h"

#include "ki/GL.h"
#include "ki/RenderClock.h"

#include "gui/Input.h"

class Node;
class EntityRegistry;
class RenderContext;

class NodeController
{
public:
    NodeController();

    virtual void prepare(
        const Assets& assets,
        EntityRegistry& entityRegistry,
        Node& node);

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

