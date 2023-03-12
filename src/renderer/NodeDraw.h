#pragma once

#include <functional>

#include "asset/Assets.h"

#include "GBuffer.h"

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

    void update(const RenderContext& ctx);

    void clear(
        const RenderContext& ctx,
        const glm::vec4& clearColor);

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
        FrameBuffer* targetBuffer,
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
        bool useGBuffer,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

    void drawBlendedImpl(
        const RenderContext& ctx,
        std::function<bool(const MeshType*)> typeSelector,
        std::function<bool(const Node*)> nodeSelector);

    void drawQuad(const RenderContext& ctx);

    void prepareQuad();

private:
    GBuffer m_gbuffer;

    GLuint m_quadVAO{ 0 };
    GLuint m_quadVBO{ 0 };

    Program* m_deferredProgram{ nullptr };
};

