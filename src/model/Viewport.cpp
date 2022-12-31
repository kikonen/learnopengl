#include "Viewport.h"

#include <functional>

#include "scene/RenderContext.h"

Viewport::Viewport(
    const std::string_view& name,
    const glm::vec3& pos,
    const glm::vec3& rotation,
    const glm::vec2& size,
    unsigned int textureID,
    Shader* shader,
    std::function<void(Viewport&)> binder)
    : m_name(name),
    m_pos(pos),
    m_rotation(rotation),
    m_size(size),
    m_textureID(textureID),
    m_shader(shader),
    m_binder(binder)
{
}

Viewport::~Viewport()
{
    KI_INFO_SB("VIEW_PORT: delete");
}

void Viewport::setTextureID(unsigned int textureID)
{
    m_textureID = textureID;
}

void Viewport::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_shader->prepare(assets);

    prepareVBO();
}

void Viewport::prepareVBO()
{
    m_vao.create();
    m_vbo.create();

    const float x = m_pos.x;
    const float y = m_pos.y;
    const float z = m_pos.z;

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
        glNamedBufferStorage(m_vbo, sizeof(vertices), &vertices, 0);

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

void Viewport::update(const RenderContext& ctx)
{
}

void Viewport::bind(const RenderContext& ctx)
{
    if (m_textureID == 0) return;

    m_shader->bind(ctx.state);

    //m_shader->viewportTex.set(UNIT_VIEWPORT);
    ctx.state.bindTexture(UNIT_VIEWPORT, m_textureID, true);

    m_shader->u_effect.set((int)m_effect);

    ctx.state.bindVAO(m_vao);

    m_binder(*this);
}

void Viewport::unbind(const RenderContext& ctx)
{
    //m_shader->unbind();
}

void Viewport::draw(const RenderContext& ctx)
{
    if (m_textureID == 0) return;
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
