#pragma once

#include <functional>
#include <map>

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "BlurBuffer.h"
#include "EffectBuffer.h"
#include "TextureQuad.h"
#include "ScreenTri.h"

#include "backend/DrawOptions.h"
#include "query/TimeElapsedQuery.h"

#include "render/size.h"
#include "render/NodeCollection.h"

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Program;
class Node;

class ParticleRenderer;
class DecalRenderer;

namespace mesh {
    class MeshType;
    struct LodMesh;
}

namespace render {
    class NodeDraw final {
        friend class editor::EditorFrame;

    public:
        NodeDraw();
        ~NodeDraw();

        void prepareRT(
            const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void drawNodes(
            const RenderContext& ctx,
            FrameBuffer* dstBuffer,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            GLbitfield copyMask);

        void drawProgram(
            const RenderContext& ctx,
            const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

    private:
        void passDeferred(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* targetBuffer);

        void passDeferredPreDepth(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

        void passDeferredDraw(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

        void passDeferredCombine(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passForward(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* targetBuffer);

        void passOit(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* targetBuffer);

        void passSkybox(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passEffect(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            FrameBuffer* targetBuffer);

        void passDecal(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passParticle(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passScreenspace(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* srcBuffer,
            FrameBuffer* finalBuffer);

        void passEmission(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passFogBlend(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passOitBlend(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passBloom(
            const RenderContext& ctx,
            FrameBuffer* srcBuffer,
            FrameBuffer* finalBuffer);

        void passCopy(
            const RenderContext& ctx,
            FrameBuffer* srcBuffer,
            FrameBuffer* dstBuffer,
            GLbitfield copyMask);

        void passDebug(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passCleanup(
            const RenderContext& ctx);

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

        void drawBlendedImpl(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector);

        void drawSkybox(
            const RenderContext& ctx);

    public:
        render::NodeCollection m_collection;

    private:
        GBuffer m_gBuffer;
        OITBuffer m_oitBuffer;
        EffectBuffer m_effectBuffer;
        BlurBuffer m_blurBuffer;

        TextureQuad& m_textureQuad;
        ScreenTri& m_screenTri;

        Program* m_deferredProgram{ nullptr };
        Program* m_oitProgram{ nullptr };
        Program* m_oitBlendProgram{ nullptr };
        Program* m_bloomInitProgram{ nullptr };
        Program* m_bloomBlurProgram{ nullptr };
        Program* m_bloomFinalProgram{ nullptr };
        Program* m_emissionProgram{ nullptr };
        Program* m_fogProgram{ nullptr };
        Program* m_blurVerticalProgram{ nullptr };
        Program* m_blurHorizontalProgram{ nullptr };
        Program* m_blurFinalProgram{ nullptr };
        // Program* m_hdrGammaProgram{ nullptr };

        query::TimeElapsedQuery m_timeElapsedQuery;

        std::unique_ptr<ParticleRenderer> m_particleRenderer;
        std::unique_ptr<DecalRenderer> m_decalRenderer;

        int m_effectBloomIterations{ 0 };

        bool m_drawDebug{ false };
        bool m_glUseInvalidate{ false };

        bool m_particleEnabled{ true };
        bool m_decalEnabled{ true };

        bool m_prepassDepthEnabled{ false };
        bool m_effectOitEnabled{ true };
        bool m_effectEmissionEnabled{ true };
        bool m_effectFogEnabled{ true };
        bool m_effectBloomEnabled{ true };
    };
}
