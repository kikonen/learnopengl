#pragma once

#include "asset/Assets.h"

#include "ki/GL.h"

class Node;
class Registry;
class RenderContext;

//
// Generate node OR entity instances for node
//
class Generator
{
public:
    Generator() = default;
    virtual ~Generator() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) {}

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) {}

protected:
    size_t m_poolSize = 0;
};
