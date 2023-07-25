#pragma once

#include <functional>

#include "asset/Assets.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "EffectBuffer.h"
#include "PlainQuad.h"
#include "TextureQuad.h"

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

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        GLbitfield copyMask);

    void drawDebug(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer);

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

private:
    GBuffer m_gBuffer;
    OITBuffer m_oitBuffer;
    EffectBuffer m_effectBuffer;

    PlainQuad m_plainQuad;
    TextureQuad m_textureQuad;

    Program* m_deferredProgram{ nullptr };
    Program* m_oitProgram{ nullptr };
    Program* m_blendOitProgram{ nullptr };
    Program* m_bloomProgram{ nullptr };
    Program* m_blendBloomProgram{ nullptr };
    Program* m_emissionProgram{ nullptr };
    Program* m_fogProgram{ nullptr };
};
