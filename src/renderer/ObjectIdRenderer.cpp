#include "ObjectIdRenderer.h"

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

    const auto& res = ctx.m_resolution;

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

    m_idShader = shaders.getShader(assets, TEX_OBJECT_ID, { { DEF_USE_ALPHA, "1"} });
    m_idShader->prepare(assets);

    m_idShaderSprite = shaders.getShader(assets, TEX_OBJECT_ID_SPRITE, { { DEF_USE_ALPHA, "1"} });
    m_idShaderSprite->prepare(assets);

    m_debugViewport = std::make_shared<Viewport>(
        "ObjectID",
        glm::vec3(-1.0, 1.0, 0),
        //glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        0,
        shaders.getShader(assets, TEX_VIEWPORT));

    m_debugViewport->prepare(assets);
}

void ObjectIdRenderer::update(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;
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

    m_debugViewport->setTextureID(m_idBuffer->m_spec.attachments[0].textureID);
}

void ObjectIdRenderer::render(
    const RenderContext& ctx)
{
    RenderContext idCtx("OBJECT_ID", &ctx, ctx.m_camera, m_idBuffer->m_spec.width, m_idBuffer->m_spec.height);

    m_idBuffer->bind(idCtx);

    drawNodes(idCtx);
    m_idBuffer->unbind(ctx);
}

void ObjectIdRenderer::drawNodes(const RenderContext& ctx)
{
    ctx.state.enable(GL_DEPTH_TEST);

    ctx.bindDefaults();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        auto renderTypes = [this, &ctx](const MeshTypeMap& typeMap) {
            for (const auto& it : typeMap) {
                auto& type = *it.first.type;
                if (type.m_flags.noSelect) continue;

                auto shader = m_idShader;
                if (type.m_entityType == EntityType::sprite) {
                    shader = m_idShaderSprite;
                }

                auto& batch = ctx.m_batch;
                batch.m_useObjectIDBuffer = true;

                for (auto& node : it.second) {
                    batch.draw(ctx, *node, shader);
                }

                batch.m_useObjectIDBuffer = false;
            }
        };

        for (const auto& all : ctx.registry.solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : ctx.registry.alphaNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : ctx.registry.blendedNodes) {
            renderTypes(all.second);
        }

        ctx.m_batch.flush(ctx);
    }
}
