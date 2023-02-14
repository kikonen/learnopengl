#pragma once

#include "asset/Assets.h"

#include "ki/GL.h"

class Node;
class Registry;
class RenderContext;

//
// Generate node OR entity instances for node
//
class NodeGenerator
{
public:
    NodeGenerator() = default;
    virtual ~NodeGenerator() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& container) {}

    virtual void update(
        const RenderContext& ctx,
        Node& container,
        Node* containerParent) {}

protected:
    size_t m_poolSize = 0;
};
