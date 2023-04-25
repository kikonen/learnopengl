#pragma once

#include <functional>

#include "asset/Assets.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "Quad.h"

class RenderContext;
class Program;
class MeshType;
class Node;
class Program;
class Registry;

//const auto ANY_TYPE = [](const MeshType* type) { return true; };
//const auto ANY_NODE = [](const Node* node) { return true; };

class NodeDraw final {
public:
    void prepare(
        const Assets& assets,
        Registry* registry);

    void updateView(const RenderContext& ctx);

    void clear(
        const RenderContext& ctx,
        GLbitfield clearMask,
        const glm::vec4& clearColor);

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        GLbitfield clearMask,
        const glm::vec4& clearColor);

    void drawBlended(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector);

    void drawProgram(
        const RenderContext& ctx,
        Program* program,
        Program* programSprite,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector);

private:
    void drawNodesImpl(
        const RenderContext& ctx,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector);

    void drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector);

    void drawQuad(const RenderContext& ctx);

    void prepareQuad();

private:
    GBuffer m_gbuffer;
    OITBuffer m_oitbuffer;

    Quad m_quad;

    Program* m_deferredProgram{ nullptr };
    Program* m_oitProgram{ nullptr };
};
