#pragma once

#include "asset/Assets.h"

#include "ki/RenderClock.h"

#include "kigl/kigl.h"

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

        m_registry = registry;
    }

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node)
    {
        return false;
    }

    virtual void onKey(Input* input, const ki::RenderClock& clock) {};
    virtual void onMouseMove(Input* input, float xoffset, float yoffset) {};
    virtual void onMouseScroll(Input* input, float xoffset, float yoffset) {};

protected:
    bool m_prepared{ false };

    Registry* m_registry{ nullptr };
};
