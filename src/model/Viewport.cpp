#include "Viewport.h"

#include <functional>

#include "util/Util.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "render/FrameBuffer.h"

#include "render/RenderContext.h"


Viewport::Viewport(
    std::string_view name,
    const glm::vec3& pos,
    const glm::vec3& rotation,
    const glm::vec2& size,
    bool m_useDirectBlit,
    unsigned int textureId,
    Program* program)
    : m_name(name),
    m_position(pos),
    m_rotation(rotation),
    m_size(size),
    m_useDirectBlit(false && m_useDirectBlit),
    m_textureId(textureId),
    m_program(program)
{
}

Viewport::~Viewport()
{
    KI_INFO("VIEW_PORT: delete");
}

void Viewport::setSourceFrameBuffer(FrameBuffer* frameBuffer)
{
    m_sourceBuffer = frameBuffer;
}

void Viewport::setDestinationFrameBuffer(FrameBuffer * frameBuffer)
{
    m_destinationBuffer = frameBuffer;
}

void Viewport::setTextureId(GLuint textureId)
{
    m_textureId = textureId;
}

void Viewport::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    // NOTE KI no program VAO/VBO with framebuffer blit
    if (m_useDirectBlit) return;

    m_program->prepare(assets);

    prepareVBO();
}

void Viewport::prepareVBO()
{
    m_vao.create("viewport");
    m_vbo.create();

    const float x = m_position.x;
    const float y = m_position.y;
    const float z = m_position.z;

    const float w = m_size.x;
    const float h = m_size.y;

    const float vertices[] = {
        x,     y,     z, 0.0f, 1.0f,
        x,     y - h, z, 0.0f, 0.0f,
        x + w, y,     z, 1.0f, 1.0f,
        x + w, y - h, z, 1.0f, 0.0f,
    };

    // setup plane VAO
    const int vao = m_vao;
    {
        m_vbo.init(sizeof(vertices), (void*) & vertices, 0);

        constexpr int stride_size = 5 * sizeof(float);
        glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, m_vbo, 0, stride_size);

        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_FLOAT, GL_TRUE, 3 * sizeof(float));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);
    }
}

void Viewport::update(const UpdateContext& ctx)
{
}

void Viewport::bind(const RenderContext& ctx)
{
    m_bindBefore(*this);

    if (m_useDirectBlit) return;
    if (m_textureId == 0) return;

    m_program->bind(ctx.m_state);

    //m_program->viewportTex.set(UNIT_VIEWPORT);
    ctx.m_state.bindTexture(UNIT_VIEWPORT, m_textureId, true);

    m_program->u_toneHdri->set(true);
    m_program->u_gammaCorrect->set(false);

    if (m_effectEnabled) {
        m_program->u_effect->set(util::as_integer(m_effect));
    }

    ctx.m_state.bindVAO(m_vao);
    m_bindAfter(*this);
}

void Viewport::unbind(const RenderContext& ctx)
{
    //m_program->unbind();
}

void Viewport::draw(const RenderContext& ctx)
{
    if (m_useDirectBlit) {
        m_sourceBuffer->blit(m_destinationBuffer, GL_COLOR_BUFFER_BIT, m_position, m_size, GL_LINEAR);
    }
    else
    {
        if (m_textureId == 0) return;
        glEnable(GL_FRAMEBUFFER_SRGB);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
}
