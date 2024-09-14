#include "ObjectIdRenderer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/Camera.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


ki::node_id ObjectIdRenderer::getObjectId(
    const RenderContext& ctx,
    float screenPosX,
    float screenPosY,
    Viewport* mainViewport)
{
    // https://stackoverflow.com/questions/10123601/opengl-read-pixels-from-framebuffer-for-ing-rounded-up-to-255-0xff
    // https://stackoverflow.com/questions/748162/what-are-the-differences-between-a-frame-buffer-object-and-a-pixel-buffer-object

    glFlush();
    glFinish();


    unsigned char data[4];
    memset(data, 100, sizeof(data));

    constexpr float GL_SCREEN_SIZE = 2.f;

    const auto& res = ctx.m_resolution;

    const int screenW = res.x;
    const int screenH = res.y;

    // NOTE KI this logic fails if rotation is applied in viewport
    const auto& vpSize = mainViewport->getSize();
    const auto& vpPos = mainViewport->getPosition();

    const float w = screenW * (vpSize.x / GL_SCREEN_SIZE);
    const float h = screenH * (vpSize.y / GL_SCREEN_SIZE);

    const float ratioX = m_idBuffer->m_spec.width / w;
    const float ratioY = m_idBuffer->m_spec.height / h;

    const float offsetX = screenW * (vpPos.x + 1.f) / GL_SCREEN_SIZE;
    const float offsetY = screenH * (1.f - (vpPos.y + 1.f) / GL_SCREEN_SIZE);

    const float posx = (static_cast<float>(screenPosX) - offsetX) * ratioX;
    const float posy = (static_cast<float>(screenPosY) - offsetY) * ratioY;

    if (posx < 0 || posx > w || posy < 0 || posy > h) return -1;


    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, *m_idBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        //glNamedFramebufferReadBuffer(*m_idBuffer, GL_COLOR_ATTACHMENT0);

        //int readFormat;
        //glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);

        glReadPixels(
            static_cast<GLint>(posx),
            static_cast<GLint>(m_idBuffer->m_spec.height - posy),
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

        m_idBuffer->unbind(ctx);
    }

    const ki::node_id nodeId =
        data[0] +
        data[1] * 256 +
        data[2] * 256 * 256 +
        data[3] * 256 * 256 * 256;

    return nodeId;
}

void ObjectIdRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;

    m_idProgramId = ProgramRegistry::get().getProgram(SHADER_OBJECT_ID, { { DEF_USE_ALPHA, "1"} });
    Program::get(m_idProgramId)->prepareRT();

    m_debugViewport = std::make_shared<Viewport>(
        "ObjectID",
        glm::vec3(-1.0, 1.0, 0),
        //glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        true,
        0,
        ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

    m_debugViewport->prepareRT();
}

void ObjectIdRenderer::updateRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;
    const auto& res = ctx.m_resolution;

    // NOTE KI keep same scale as in gbuffer to allow glCopyImageSubData
    int w = (int)(assets.gBufferScale * res.x);
    int h = (int)(assets.gBufferScale * res.y);
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_idBuffer || w != m_idBuffer->m_spec.width || h != m_idBuffer->m_spec.height;
    if (!changed) return;

    // https://riptutorial.com/opengl/example/28872/using-pbos
    auto buffer = new render::FrameBuffer(
        "object_id",
        {
            w, h,
            {
                render::FrameBufferAttachment::getObjectId(),
                render::FrameBufferAttachment::getDepthRbo(),
            }
        });

    m_idBuffer.reset(buffer);
    m_idBuffer->prepare();

    m_debugViewport->setTextureId(m_idBuffer->m_spec.attachments[0].textureID);
    m_debugViewport->setSourceFrameBuffer(m_idBuffer.get());
}

void ObjectIdRenderer::render(
    const RenderContext& ctx)
{
    RenderContext idCtx("OBJECT_ID", &ctx, ctx.m_camera, m_idBuffer->m_spec.width, m_idBuffer->m_spec.height);
    idCtx.m_forceSolid = true;

    m_idBuffer->bind(idCtx);

    drawNodes(idCtx);
    m_idBuffer->unbind(ctx);
}

void ObjectIdRenderer::drawNodes(const RenderContext& ctx)
{
    auto& state = ctx.m_state;

    state.setEnabled(GL_DEPTH_TEST, true);

    ctx.bindDefaults();

    m_idBuffer->clearAll();

    {
        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (lodMesh.m_flags.tessellation) return (ki::program_id)nullptr;
                return lodMesh.m_idProgramId ? lodMesh.m_idProgramId : m_idProgramId;
            },
            [](const mesh::MeshType* type) { return !type->m_flags.noSelect; },
            [](const Node* node) { return true; },
            render::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
