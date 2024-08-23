#pragma once

#include "kigl/kigl.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

struct PrepareContext;
struct InputContext;
struct UpdateContext;
class Registry;

class Node;

class NodeController
{
public:
    NodeController();
    virtual ~NodeController();

    virtual void prepare(
        const PrepareContext& ctx,
        Node& node);

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node)
    {
        return false;
    }

    virtual void processInput(
        const InputContext& ctx) {};

    virtual void onMouseWheel(
        const InputContext& ctx,
        float xoffset,
        float yoffset) {};

protected:
    bool m_prepared{ false };

    Registry* m_registry{ nullptr };
};
