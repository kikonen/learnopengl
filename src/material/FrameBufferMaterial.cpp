#include "FrameBufferMaterial.h"

#include <fmt/format.h>

#include "util/debug.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramUniforms.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "backend/DrawBuffer.h"

#include "render/Batch.h"
#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "material/Material.h"
#include "material/MaterialRegistry.h"


namespace {
    constexpr int ATT_ALBEDO_INDEX = 0;

    constexpr int ID_INDEX = 1;
}

FrameBufferMaterial::FrameBufferMaterial(
    ki::sid id,
    const std::string& name)
    : MaterialUpdater{id, name},
    m_size{ 512, 512 },
    m_material{ std::make_unique<Material>() },
    m_textureQuad{ render::TextureQuad::get() }
{}

FrameBufferMaterial::~FrameBufferMaterial()
{
    if (m_samplerId) {
        glDeleteSamplers(1, &m_samplerId);
    }
}

void FrameBufferMaterial::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_material) return;

    auto programId = m_material->getProgram(MaterialProgramType::shader);
    if (!programId) return;

    {
        // NOTE KI depth is irrelevant, since this renders just one quad over buffer
        // => depth comes from quad, and thus should not need depth buffer
        auto buffer = new render::FrameBuffer(
            fmt::format("material_{}", m_material->m_name),
            {
                m_size.x, m_size.y,
                {
                    render::FrameBufferAttachment::getTextureRGBAHdr(GL_COLOR_ATTACHMENT0),
                    //render::FrameBufferAttachment::getDepthStencilRbo(),
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare();

        //m_buffer->m_spec.attachments[0].clearColor = glm::vec4(0, 1, 0, 1);
    }

    {
        m_textureQuad.prepare();
    }

    {
        // MaterialRegistry::get().registerMaterial(*m_material);

        glGenSamplers(1, &m_samplerId);

        auto& spec = m_material->textureSpec;
        glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_S, spec.wrapS);
        glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_T, spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        //glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, spec.minFilter);
        //glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, spec.magFilter);

        // https://stackoverflow.com/questions/42886835/modifying-parameters-of-bindless-resident-textures
        m_handle = glGetTextureSamplerHandleARB(m_buffer->m_spec.attachments[0].textureID, m_samplerId);
        glMakeTextureHandleResidentARB(m_handle);
    }
}

void FrameBufferMaterial::render(
    const RenderContext& ctx)
{
    if (!m_dirty) return;
    //m_dirty = false;

    if (m_frameCounter++ <= m_frameSkip) {
        return;
    }
    m_frameCounter = 0;

    if (!m_material) return;

    auto programId = m_material->getProgram(MaterialProgramType::shader);
    if (!programId) return;

    m_buffer->bind(ctx);
    m_buffer->clearAll();

    auto& state = ctx.m_state;

    glBindSampler(UNIT_CHANNEL_0, m_samplerId);

    {
        auto* program = Program::get(programId);
        program->bind();
        program->m_uniforms->u_materialIndex.set(m_material->m_registeredIndex);
    }

    {
        m_textureQuad.draw();
    }
}

GLuint64 FrameBufferMaterial::getTexHandle(TextureType type) const noexcept
{
    if (type == TextureType::diffuse) {
        return m_handle;
        //return m_samplerId;
    }
    return 0;
}

void FrameBufferMaterial::setMaterial(const Material* src) noexcept
{
    if (!src) {
        m_material.reset();
        return;
    }

    if (!m_material) {
        m_material = std::make_unique<Material>();
    }
    *m_material = *src;
}