#include "Viewport.h"

#include <functional>

#include <glm/ext.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Util.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "engine/UpdateContext.h"

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
    const glm::vec3& degrees,
    const glm::vec2& size,
    bool m_useDirectBlit,
    unsigned int textureId,
    Program* program)
    : m_name(name),
    m_position(pos),
    m_degreesRotation(degrees),
    m_size(size),
    m_useDirectBlit(false && m_useDirectBlit),
    m_textureId(textureId),
    m_program(program)
{
    const glm::vec3 origPosition{ m_position };
    const glm::vec3 origDegrees{ m_degreesRotation };

    setUpdate([this, origPosition, origDegrees](Viewport& vp, const UpdateContext& ctx) {
        glm::vec3 rot{ origDegrees};
        rot.y = 5.f * sinf(static_cast<float>(ctx.m_clock.ts));

        glm::vec3 position{ origPosition };
        position.z += 0.001f;

        //setDegreesRotation(rot);
        //setPosition(position);
    });
}

Viewport::~Viewport()
{
    KI_INFO(fmt::format("VIEW_PORT: delete, name={}", m_name));
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
}

void Viewport::updateTransform(const UpdateContext& ctx)
{
    if (!m_dirty) return;
    m_dirty = false;

    {
        glm::mat4 translate{ 1.f };
        glm::mat4 rotate{ 1.f };
        glm::mat4 scale{ 1.f };

        glm::vec3 pos{ m_position };
        pos.x += m_size.x / 2.f;
        pos.y -= m_size.y / 2.f;

        auto& vec = translate[3];
        vec[0] = pos.x;
        vec[1] = pos.y;
        vec[2] = pos.z;

        rotate = glm::toMat4(glm::quat(glm::radians(m_degreesRotation)));

        scale[0][0] = m_size.x / 2.f;
        scale[1][1] = m_size.y / 2.f;
        scale[2][2] = 1.f;

        m_transformMatrix = translate * rotate * scale;
    }
    {
        glm::mat4 projection = glm::perspective(
            glm::radians(90.f),
            //ctx.m_aspectRatio,
            1.f,
            0.1f,
            10.f);

        glm::mat4 view;
        {
            float z = 1.f;
            glm::vec3 pos{ 0.f, 0.f, z };
            glm::vec3 front{ 0.f, 0.f, -z };
            glm::vec3 up{ 0.f, 1.f, 0.f };

            view = glm::lookAt(
                pos,
                pos + front,
                up);
        }

        m_projected = projection * view;
    }
}

void Viewport::update(const UpdateContext& ctx)
{
    m_update(*this, ctx);
    updateTransform(ctx);
}

void Viewport::bind(const RenderContext& ctx)
{
    m_bindBefore(*this);

    if (m_useDirectBlit) return;
    if (m_textureId == 0) return;

    m_program->bind(ctx.m_state);

    ctx.m_state.bindTexture(UNIT_VIEWPORT, m_textureId, true);

    m_program->u_toneHdri->set(true);
    m_program->u_gammaCorrect->set(m_hardwareGamma ? false : m_gammaCorrect);

    glm::mat4 transformed = m_projected * m_transformMatrix;

    m_program->u_viewportTransform->set(transformed);

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
        if (!m_sourceBuffer || !m_destinationBuffer) return;

        m_sourceBuffer->blit(m_destinationBuffer, GL_COLOR_BUFFER_BIT, m_position, m_size, GL_LINEAR);
    }
    else
    {
        if (m_textureId == 0) return;

        if (m_gammaCorrect && m_hardwareGamma) {
            glEnable(GL_FRAMEBUFFER_SRGB);
            getSharedQuad().draw(ctx.m_state);
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
        else {
            getSharedQuad().draw(ctx.m_state);
        }
    }
}
