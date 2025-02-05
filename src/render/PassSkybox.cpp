#include "PassSkybox.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "model/Node.h"
#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
}

namespace render
{
    PassSkybox::PassSkybox()
        : Pass("PassSkybox")
    {
    }

    PassSkybox::~PassSkybox() = default;

    void PassSkybox::prepare(const PrepareContext& ctx)
    {
    }

    void PassSkybox::updateRT(const UpdateViewContext& ctx)
    {
        if (!updateSize(ctx)) return;
    }

    void PassSkybox::initRender(const RenderContext& ctx)
    {
        m_enabled = true;
    }

    PassContext PassSkybox::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        src.buffer->bind(ctx);
        drawSkybox(ctx);

        return src;
    }

    void PassSkybox::drawSkybox(
        const RenderContext& ctx)
    {
        auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
        auto* node = nodeRegistry.m_skybox.toNode();
        if (!node) return;

        auto* type = node->m_typeHandle.toType();

        if (!type->isReady()) return;
        if (type->m_layer != ctx.m_layer) return;

        auto* lodMesh = type->getLodMesh(0);
        auto* program = Program::get(lodMesh->m_programId);

        auto& state = ctx.m_state;

        // USE depth, but don't update
        state.setDepthFunc(GL_LEQUAL);
        state.setDepthMask(GL_FALSE);

        state.frontFace(GL_CCW);

        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SKYBOX, STENCIL_SKYBOX, ~STENCIL_OIT));
        state.polygonFrontAndBack(GL_FILL);

        program->bind();
        m_textureQuad.draw();

        state.setDepthFunc(ctx.m_depthFunc);
        state.setDepthMask(GL_TRUE);
    }
}
