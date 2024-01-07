#pragma once

#include "ki/RenderClock.h"

#include "kigl/kigl.h"

#include "gui/Input.h"

struct PrepareContext;
struct UpdateContext;
class Registry;

class Node;

class NodeController
{
public:
    NodeController() = default;
    virtual ~NodeController() = default;

    virtual void prepare(
        const PrepareContext& ctx,
        Node& node);

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
