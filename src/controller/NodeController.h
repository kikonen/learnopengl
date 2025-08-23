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
    NodeController(
        bool pawn,
        bool camera);

    virtual ~NodeController();

    bool isPawn() const noexcept
    {
        return m_pawn;
    }

    bool isCamera() const noexcept
    {
        return m_camera;
    }

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
    const bool m_pawn;
    const bool m_camera;

    bool m_prepared{ false };

    Registry* m_registry{ nullptr };
};
