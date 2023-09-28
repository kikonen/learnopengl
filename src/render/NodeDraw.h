#pragma once

#include <functional>

#include "asset/Assets.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "EffectBuffer.h"
#include "PlainQuad.h"
#include "TextureQuad.h"

#include "query/TimeElapsedQuery.h"

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
    static const unsigned int KIND_SOLID{1 << 0};
    static const unsigned int KIND_SPRITE{ 1 << 1 };
    static const unsigned int KIND_ALPHA{ 1 << 2 };
    static const unsigned int KIND_BLEND{ 1 << 3 };
    static const unsigned int KIND_ALL{ KIND_SOLID | KIND_SPRITE | KIND_ALPHA | KIND_BLEND };

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
        unsigned int kindBits,
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
        Program* programPointSprite,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        unsigned int kindBits);

private:
    bool drawNodesImpl(
        const RenderContext& ctx,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        unsigned int kindBits);

    void drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector);

    void drawSkybox(
        const RenderContext& ctx);

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
    Program* m_hdrGammaProgram{ nullptr };

    TimeElapsedQuery m_timeElapsedQuery;
};
