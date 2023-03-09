#pragma once

#include <functional>

#include "asset/Assets.h"

#include "GBuffer.h"

class RenderContext;
class Program;
class MeshType;
class Node;

//const auto ANY_TYPE = [](const MeshType* type) { return true; };
//const auto ANY_NODE = [](const Node* node) { return true; };

class NodeDraw final {
public:
    void prepare(const Assets& assets);
    void update(const RenderContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        bool includeBlended,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector,
        bool clearTarget,
        const glm::vec4& clearColor);

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

private:
    void drawNodesImpl(
        const RenderContext& ctx,
        bool includeBlended,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

    void drawBlendedImpl(
        const RenderContext& ctx,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

private:
    GBuffer m_gbuffer;
};

