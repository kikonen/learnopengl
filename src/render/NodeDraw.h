#pragma once

#include <functional>
#include <map>

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "GBuffer.h"
#include "OITBuffer.h"
#include "EffectBuffer.h"
#include "PlainQuad.h"
#include "TextureQuad.h"

#include "backend/DrawOptions.h"
#include "query/TimeElapsedQuery.h"

#include "render/size.h"

class Program;

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Node;

namespace mesh {
    class MeshType;
    struct LodMesh;
}

namespace render {
    struct PiorityKey {
        int m_priority;

        operator int() const { return m_priority; }
    };

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
    using PiorityTypeMap = std::map<PiorityKey, MeshTypeMap>;

    class NodeDraw final {
    public:
        NodeDraw();
        ~NodeDraw();

        void prepareRT(
            const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void handleNodeAdded(Node* node);

        void drawNodes(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            unsigned int kindBits,
            GLbitfield copyMask);

        void drawDebug(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer);

        void drawBlended(
            const RenderContext& ctx,
            FrameBuffer* targetBuffer,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector);

        void drawProgram(
            const RenderContext& ctx,
            const std::function<Program* (const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            unsigned int kindBits);

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<Program* (const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            unsigned int kindBits);

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

        PlainQuad m_plainQuad;
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

        // NodeDraw
        PiorityTypeMap m_solidNodes;
        // NodeDraw
        PiorityTypeMap m_alphaNodes;
        // NodeDraw
        PiorityTypeMap m_blendedNodes;
        // OBSOLETTE
        PiorityTypeMap m_invisibleNodes;
    };
}
