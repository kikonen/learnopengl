#pragma once

#include "asset/Assets.h"

#include "ki/GL.h"
#include "ki/RenderClock.h"

#include "gui/Input.h"

class Node;
class Registry;
class UpdateContext;

class NodeController
{
public:
    NodeController() = default;
    virtual ~NodeController() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node)
    {
        if (m_prepared) return;
        m_prepared = true;
    }

    virtual bool update(
        const UpdateContext& ctx,
        Node& node,
        Node* parent)
    {
        return false;
    }

    virtual void onKey(Input* input, const ki::RenderClock& clock) {};
    virtual void onMouseMove(Input* input, double xoffset, double yoffset) {};
    virtual void onMouseScroll(Input* input, double xoffset, double yoffset) {};

protected:
    bool m_prepared = false;
};
