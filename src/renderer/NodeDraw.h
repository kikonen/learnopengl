#pragma once

#include <functional>

class RenderContext;
class Program;
class MeshType;
class Node;

class NodeDraw final {
public:
    void drawNodes(
        const RenderContext& ctx,
        bool includeBlended,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

    void drawBlended(
        const RenderContext& ctx,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

    void drawProgram(
        const RenderContext& ctx,
        Program* program,
        Program* programSprite,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);
};

