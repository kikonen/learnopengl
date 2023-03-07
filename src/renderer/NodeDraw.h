#pragma once

#include <functional>

class RenderContext;
class Program;
class MeshType;
class Node;

//const auto ANY_TYPE = [](const MeshType* type) { return true; };
//const auto ANY_NODE = [](const Node* node) { return true; };

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

