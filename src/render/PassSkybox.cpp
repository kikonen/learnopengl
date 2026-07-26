#include "PassSkybox.h"

#include "kigl/GLState.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "model/Node.h"

#include "engine/Engine.h"
#include "scene/Scene.h"
#include "scene/Skybox.h"
#include "scene/World.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    const std::string PROGRAM_NAME = "skybox";
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
        m_program = Program::get(ProgramRegistry::get().getProgram(PROGRAM_NAME));
        // scenes without a day/night cycle have no night skybox; that variant omits the
        // UNIT_SKYBOX_NIGHT sampler so it isn't validated against an unbound texture unit
        m_programNight = Program::get(ProgramRegistry::get().getProgram(
            PROGRAM_NAME, { { DEF_USE_SKYBOX_NIGHT, "1" } }));
    }

    void PassSkybox::updateView(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassSkybox::initRender(const RenderContext& ctx)
    {
        auto& state = ctx.getGLState();
        const auto& dbg = ctx.getDebug();

        m_enabled = dbg.m_skyboxEnabled;
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
        auto* scene = ctx.getRegistry()->getEngine().getCurrentScene().get();
        if (!scene) return;

        auto* skybox= scene->getSkybox().get();
        if (!skybox) return;

        auto* skyboxMaterial = skybox->getMaterial(0).get();
        if (!skyboxMaterial) return;

        if (ctx.m_layer != 1) return;

        // night skybox (material 1) only exists in day/night-cycle scenes; pick the shader
        // variant accordingly so the day-only case never declares the UNIT_SKYBOX_NIGHT sampler
        const bool hasNight = skybox->getMaterial(1).get() != nullptr;
        auto* program = hasNight ? m_programNight : m_program;

        // 0 = full day .. 1 = full night, from the World day-night model (RT-published)
        float skyBlend = 0.f;
        if (auto* world = scene->getWorld().get()) {
            skyBlend = world->getSkyBlend();
        }

        auto& state = ctx.getGLState();

        // NOTE KI cannot update stencil without depth update
        // => thus *UPDATE* depth
        // => stencil mask is used in fog pass
        state.setDepthFunc(GL_LEQUAL);
        state.frontFace(GL_CCW);

        state.setBlendMode({});
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SKYBOX, STENCIL_SKYBOX, ~STENCIL_OIT));
        state.polygonFrontAndBack(GL_FILL);

        program->bind();
        if (hasNight) {
            program->setFloat("u_skyBlend", skyBlend);
        }
        m_textureQuad.draw();

        state.setDepthFunc(ctx.m_depthFunc);
        state.setDepthMask(GL_TRUE);
    }
}
