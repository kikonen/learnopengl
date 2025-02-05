#pragma once

#include <functional>

#include "query/TimeElapsedQuery.h"

#include "render/size.h"

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Program;

namespace editor {
    class EditorFrame;
}

namespace mesh {
    struct LodMesh;
}

namespace render {

    struct DrawContext;
    struct PassContext;

    class FrameBuffer;

    class PassDeferred;
    class PassForward;
    class PassDecal;
    class PassParticle;
    class PassEffect;
    class PassFog;
    class PassOit;
    class PassBloom;
    class PassSkybox;
    class PassDebug;
    class PassCopy;

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
            const DrawContext& drawContext,
            FrameBuffer* dstBuffer);

        void drawProgram(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector);

    private:
        void passCleanup(
            const RenderContext& ctx);

    private:
        query::TimeElapsedQuery m_timeElapsedQuery;

        std::unique_ptr<render::PassDeferred> m_passDeferred;
        std::unique_ptr<render::PassForward> m_passForward;
        std::unique_ptr<render::PassDecal> m_passDecal;
        std::unique_ptr<render::PassParticle> m_passParticle;
        std::unique_ptr<render::PassEffect> m_passEffect;
        std::unique_ptr<render::PassFog> m_passFog;
        std::unique_ptr<render::PassOit> m_passOit;
        std::unique_ptr<render::PassBloom> m_passBloom;
        std::unique_ptr<render::PassSkybox> m_passSkybox;
        std::unique_ptr<render::PassDebug> m_passDebug;
        std::unique_ptr<render::PassCopy> m_passCopy;

        bool m_glUseInvalidate{ false };
    };
}
