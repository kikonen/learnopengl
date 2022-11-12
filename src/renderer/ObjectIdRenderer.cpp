#include "ObjectIdRenderer.h"

#include "asset/ShaderBind.h"

ObjectIdRenderer::ObjectIdRenderer()
    : Renderer()
{
}

ObjectIdRenderer::~ObjectIdRenderer()
{
}

int ObjectIdRenderer::getObjectId(
    const RenderContext& ctx,
    double screenPosX,
    double screenPosY,
    Viewport* mainViewport)
{
    // https://stackoverflow.com/questions/10123601/opengl-read-pixels-from-framebuffer-for-ing-rounded-up-to-255-0xff
    // https://stackoverflow.com/questions/748162/what-are-the-differences-between-a-frame-buffer-object-and-a-pixel-buffer-object

    glFlush();
    glFinish();


    unsigned char data[4];
    memset(data, 100, sizeof(data));

    constexpr float GL_SCREEN_SIZE = 2.f;

    const auto& res = ctx.resolution;

    int screenW = res.x;
    int screenH = res.y;

    float w = screenW * (mainViewport->m_size.x / GL_SCREEN_SIZE);
    float h = screenH * (mainViewport->m_size.y / GL_SCREEN_SIZE);

    float ratioX = m_idBuffer->m_spec.width / w;
    float ratioY = m_idBuffer->m_spec.height / h;

    float offsetX = screenW * (mainViewport->m_pos.x + 1.f) / GL_SCREEN_SIZE;
    float offsetY = screenH * (1.f - (mainViewport->m_pos.y + 1.f) / GL_SCREEN_SIZE);

    float posx = (screenPosX - offsetX) * ratioX;
    float posy = (screenPosY - offsetY) * ratioY;

    if (posx < 0 || posx > w || posy < 0 || posy > h) return -1;


    {
        m_idBuffer->bind(ctx);

        glReadBuffer(GL_COLOR_ATTACHMENT0);

        int readFormat;
        glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);

        glReadPixels(posx, m_idBuffer->m_spec.height - posy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

        int x = 0;
        m_idBuffer->unbind(ctx);
    }

    int objectID =
        data[0] +
        data[1] * 256 +
        data[2] * 256 * 256;

    return objectID;
}

void ObjectIdRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    m_idShader = shaders.getShader(assets, TEX_OBJECT_ID);
    m_idShader->prepare(assets);

    m_idShaderAlpha = shaders.getShader(assets, TEX_OBJECT_ID, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1"} });
    m_idShaderAlpha->prepare(assets);

    m_idShaderSprite = shaders.getShader(assets, TEX_OBJECT_ID_SPRITE, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1"} });
    m_idShaderSprite->prepare(assets);

    debugViewport = std::make_shared<Viewport>(
        glm::vec3(-1.0, 1.0, 0),
        //glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        0,
        shaders.getShader(assets, TEX_VIEWPORT));

    debugViewport->prepare(assets);
}

void ObjectIdRenderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
    const auto& res = ctx.resolution;
    int w = ctx.assets.resolutionScale.x * res.x;
    int h = ctx.assets.resolutionScale.y * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_idBuffer || w != m_idBuffer->m_spec.width || h != m_idBuffer->m_spec.height;
    if (!changed) return;

    // https://riptutorial.com/opengl/example/28872/using-pbos
    auto buffer = new TextureBuffer({
        w, h,
        { FrameBufferAttachment::getObjectId(), FrameBufferAttachment::getRBODepth() } });

    m_idBuffer.reset(buffer);
    m_idBuffer->prepare(true, { 0, 0, 0, 0.5 });

    debugViewport->setTextureID(m_idBuffer->m_spec.attachments[0].textureID);
}

void ObjectIdRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    RenderContext idCtx("OBJECT_ID", &ctx, ctx.camera, m_idBuffer->m_spec.width, m_idBuffer->m_spec.height);

    m_idBuffer->bind(idCtx);

    drawNodes(idCtx, registry);
    m_idBuffer->unbind(ctx);
}

void ObjectIdRenderer::drawNodes(const RenderContext& ctx, const NodeRegistry& registry)
{
    ctx.state.enable(GL_DEPTH_TEST);

    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    ctx.state.enable(GL_CULL_FACE);
    ctx.state.cullFace(GL_BACK);
    ctx.state.frontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        auto renderTypes = [this, &ctx](const NodeTypeMap& typeMap) {
            for (const auto& it : typeMap) {
                auto& type = *it.first;
                if (type.m_flags.noSelect) continue;

                auto shader = type.m_flags.alpha ? m_idShaderAlpha : m_idShader;
                if (type.m_flags.sprite) {
                    shader = m_idShaderSprite;
                }

                ShaderBind bound(shader);

                auto& batch = ctx.m_batch;
                batch.m_objectIDBuffer = true;

                type.bind(ctx, bound.shader);
                batch.bind(ctx, bound.shader);

                for (auto& node : it.second) {
                    batch.draw(ctx, *node, bound.shader);
                }

                batch.flush(ctx, type);
                type.unbind(ctx);
                batch.m_objectIDBuffer = false;
            }
        };

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : registry.alphaNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second);
        }
    }
}
