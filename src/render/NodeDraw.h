#pragma once

#include <functional>
#include <map>

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "EffectBuffer.h"
#include "TextureQuad.h"

#include "backend/DrawOptions.h"
#include "query/TimeElapsedQuery.h"

#include "render/size.h"

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Program;
class Node;

class ParticleRenderer;
class DecalRenderer;

namespace editor {
    class EditorFrame;
}

namespace mesh {
    class MeshType;
    struct LodMesh;
}

namespace render {
    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    struct MeshTypeKey {
        // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
        MeshTypeKey(pool::TypeHandle typeHandle)
            : m_typeHandle(typeHandle)
        {}

        bool operator<(const MeshTypeKey& o) const;

        const pool::TypeHandle m_typeHandle;
    };

    using NodeVector = std::vector<pool::NodeHandle>;
    using MeshTypeMap = std::map<MeshTypeKey, NodeVector>;

    class NodeDraw final {
        friend class editor::EditorFrame;

    public:
        NodeDraw();
        ~NodeDraw();

        void prepareRT(
            const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void handleNodeAdded(Node* node);

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

        void passEffectBlend(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* targetBuffer);

        void passDecalBlend(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passParticleBlend(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void passScreenspace(
            const RenderContext& ctx,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits,
            FrameBuffer* srcBuffer,
            FrameBuffer* finalBuffer);

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
        void drawBlended(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector);

    private:
        void insertNode(
            MeshTypeMap* map,
            Node* node);

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

    private:
        GBuffer m_gBuffer;
        OITBuffer m_oitBuffer;
        EffectBuffer m_effectBuffer;

        TextureQuad& m_textureQuad;

        Program* m_deferredProgram{ nullptr };
        Program* m_oitProgram{ nullptr };
        Program* m_blendOitProgram{ nullptr };
        Program* m_bloomProgram{ nullptr };
        Program* m_blendBloomProgram{ nullptr };
        Program* m_emissionProgram{ nullptr };
        Program* m_fogProgram{ nullptr };
        Program* m_hdrGammaProgram{ nullptr };

        query::TimeElapsedQuery m_timeElapsedQuery;

        std::unique_ptr<ParticleRenderer> m_particleRenderer;
        std::unique_ptr<DecalRenderer> m_decalRenderer;

        // NodeDraw
        MeshTypeMap m_solidNodes;
        // NodeDraw
        MeshTypeMap m_alphaNodes;
        // NodeDraw
        MeshTypeMap m_blendedNodes;
        // OBSOLETTE
        MeshTypeMap m_invisibleNodes;

        int m_effectBloomIterations{ 0 };
        bool m_drawDebug{ false };
        bool m_glUseInvalidate{ false };
        bool m_prepassDepthEnabled{ false };
        bool m_effectOitEnabled{ true };
        bool m_effectFogEnabled{ true };
        bool m_effectBloomEnabled{ true };
    };
}
