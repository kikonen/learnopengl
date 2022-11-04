#include "Viewport.h"

#include <functional>

const int ATTR_VIEW_POS = 0;
const int ATTR_VIEW_TEX = 1;

Viewport::Viewport(
    const glm::vec3& pos, 
    const glm::vec3& rotation, 
    const glm::vec2& size, 
    unsigned int textureID, 
    Shader* shader,
    std::function<void(Viewport&)> binder)
    : pos(pos), rotation(rotation), size(size), textureID(textureID), shader(shader), binder(binder)
{
}

Viewport::~Viewport()
{
    KI_INFO_SB("VIEW_PORT: delete");
}

void Viewport::setTextureID(unsigned int textureID)
{
    this->textureID = textureID;
}

void Viewport::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    shader->prepare(assets);

    buffers.prepare(false);

    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    float w = size.x;
    float h = size.y;

    float vertices[] = {
        x,     y,     z, 0.0f, 1.0f,
        x,     y - h, z, 0.0f, 0.0f,
        x + w, y,     z, 1.0f, 1.0f,
        x + w, y - h, z, 1.0f, 0.0f,
    };

    // setup plane VAO
    const int vao = buffers.VAO;
    {
        glNamedBufferStorage(buffers.VBO, sizeof(vertices), &vertices, 0);

        constexpr int stride_size = 5 * sizeof(float);
        glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, buffers.VBO, 0, stride_size);

        glEnableVertexArrayAttrib(vao, ATTR_VIEW_POS);
        glEnableVertexArrayAttrib(vao, ATTR_VIEW_TEX);

        glVertexArrayAttribFormat(vao, ATTR_VIEW_POS, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(vao, ATTR_VIEW_TEX, 2, GL_FLOAT, GL_TRUE, 3 * sizeof(float));

        glVertexArrayAttribBinding(vao, ATTR_VIEW_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_VIEW_TEX, VBO_VERTEX_BINDING);
    }
}

void Viewport::update(const RenderContext& ctx)
{
}

void Viewport::bind(const RenderContext& ctx)
{
    if (textureID == 0) return;

    shader->bind();

    // TODO KI this is bogus; reserve some high-up unit index
    const int unitIndex = 0;

    ctx.state.bindTexture(unitIndex, textureID);
    shader->viewportTex.set(unitIndex);

    shader->effect.set((int)effect);

    glBindVertexArray(buffers.VAO);

    binder(*this);
}

void Viewport::unbind(const RenderContext& ctx)
{
    shader->unbind();
}

void Viewport::draw(const RenderContext& ctx)
{
    if (textureID == 0) return;
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
