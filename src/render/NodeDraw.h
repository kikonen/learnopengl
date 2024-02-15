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

class Program;

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Node;

namespace mesh {
    class MeshType;
}

namespace render {
    //const auto ANY_TYPE = [](const mesh::MeshType* type) { return true; };
    //const auto ANY_NODE = [](const Node* node) { return true; };

    //
    // NOTE KI program key is REQUIRED for sorting "gull back face" draws
    // next to each other to avoid redundant state changes
    // => relies into fact that std::map is sorted by this
    //
    struct ProgramKey {
        ProgramKey(
            ki::program_id programID,
            int typePriority,
            const backend::DrawOptions& drawOptions) noexcept
            : programID(programID),
            typePriority(typePriority),
            renderBack(drawOptions.m_renderBack),
            wireframe(drawOptions.m_wireframe)
        {};

        std::string str() const noexcept
        {
            return fmt::format(
                "<PROGRAM_KEY: id={}, pri={}, renderBack={}, wireframe={}>",
                programID, typePriority, renderBack, wireframe);
        }

        bool operator<(const ProgramKey& o) const noexcept {
            // NOTE KI renderBack & wireframe goes into separate render always due to GL state
            // => reduce state changes via sorting
            return std::tie(typePriority, programID, renderBack, wireframe) <
                std::tie(o.typePriority, o.programID, o.renderBack, o.wireframe);
        }

        const int typePriority;
        const ki::program_id programID;
        const bool renderBack;
        const bool wireframe;
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
    using ProgramTypeMap = std::map<ProgramKey, MeshTypeMap>;

    class NodeDraw final {
    public:
        static const unsigned int KIND_SOLID{ 1 << 0 };
        static const unsigned int KIND_SPRITE{ 1 << 1 };
        static const unsigned int KIND_ALPHA{ 1 << 2 };
        static const unsigned int KIND_BLEND{ 1 << 3 };
        static const unsigned int KIND_ALL{ KIND_SOLID | KIND_SPRITE | KIND_ALPHA | KIND_BLEND };

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
            const std::function<Program* (const mesh::MeshType*)>& programSelector,
            const std::function<bool(const mesh::MeshType*)>& typeSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            unsigned int kindBits);

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
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
        TextureQuad m_textureQuad;

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
        ProgramTypeMap m_solidNodes;
        // NodeDraw
        ProgramTypeMap m_alphaNodes;
        // NodeDraw
        ProgramTypeMap m_spriteNodes;
        // NodeDraw
        ProgramTypeMap m_blendedNodes;
        // OBSOLETTE
        ProgramTypeMap m_invisibleNodes;
    };
}
