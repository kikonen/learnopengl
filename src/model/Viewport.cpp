#include "Viewport.h"

#include <functional>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Util.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "render/FrameBuffer.h"

#include "render/RenderContext.h"
#include "render/TextureQuad.h"


namespace {
    TextureQuad g_sharedQuad;

    bool g_sharedQuadPrepared{ false };
    TextureQuad& getSharedQuad() {
        if (!g_sharedQuadPrepared) {
            g_sharedQuadPrepared = true;
            g_sharedQuad.prepare();
        }
        return g_sharedQuad;
    }
}

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

    prepareTransform();
}

void Viewport::prepareTransform()
{
    glm::mat4 translate{ 1.f };
    glm::mat4 rotate{ 1.f };
    glm::mat4 scale{ 1.f };

    glm::vec3 pos{ -1.0f, 1.f, 0.f };
    pos -= m_position;

    auto& vec = translate[3];
    vec[0] = pos.x;
    vec[1] = pos.y;
    vec[2] = pos.z;

    rotate = glm::toMat4(glm::quat(glm::radians(m_rotation)));

    scale[0][0] = m_size.x;
    scale[1][1] = m_size.y;
    scale[2][2] = 1.f;

    m_transformMatrix = translate * rotate * scale;
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

    ctx.m_state.bindTexture(UNIT_VIEWPORT, m_textureId, true);

    m_program->u_toneHdri->set(true);
    m_program->u_gammaCorrect->set(false);
    m_program->u_viewportTransform->set(m_transformMatrix);

    if (m_effectEnabled) {
        m_program->u_effect->set(util::as_integer(m_effect));
    }

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
        getSharedQuad().draw(ctx.m_state);
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
}
