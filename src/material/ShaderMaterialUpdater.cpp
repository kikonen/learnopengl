#include "ShaderMaterialUpdater.h"

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
#include "render/TextureQuad.h"

#include "material/Material.h"
#include "material/MaterialRegistry.h"

namespace {
    constexpr int ATT_ALBEDO_INDEX = 0;

    constexpr int ID_INDEX = 1;
}

ShaderMaterialUpdater::ShaderMaterialUpdater(
    ki::material_updater_id id,
    const std::string& name)
    : MaterialUpdater{id, name},
    m_size{ 512, 512 }
{}

ShaderMaterialUpdater::~ShaderMaterialUpdater()
{
    if (m_samplerId) {
        glDeleteSamplers(1, &m_samplerId);
    }
}

void ShaderMaterialUpdater::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_material) return;

    auto programId = m_material->getProgram(material::ProgramType::shader);
    if (!programId) return;

    {
        // NOTE KI depth is irrelevant, since this renders just one quad over buffer
        // => depth comes from quad, and thus should not need depth buffer
        m_frameBuffer = util::Ref<render::FrameBuffer>::create(
            fmt::format("material_{}", m_material->m_name),
            render::FrameBufferSpecification {
                m_size.x, m_size.y,
                {
                    render::FrameBufferAttachment::getTextureRGBA(GL_COLOR_ATTACHMENT0),
                    //render::FrameBufferAttachment::getDepthStencilRbo(),
                }
            });
        m_frameBuffer->prepare();

        //m_frameBuffer->m_spec.attachments[0].clearColor = glm::vec4(0, 1, 0, 1);
    }

    prepareTexture();
}

void ShaderMaterialUpdater::prepareTexture()
{
    const auto& assets = Assets::get();
    if (assets.drawUseArrayTexture) {
        prepareSingle();
    }
    else {
        prepareArray();
    }
}

void ShaderMaterialUpdater::prepareSingle()
{
    // MaterialRegistry::get().registerMaterial(*m_material);

    glGenSamplers(1, &m_samplerId);

    auto& spec = m_material->defaultTextureSpec;
    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_S, spec.asWrapS());
    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_T, spec.asWrapT());

    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
    //glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, spec.asMinFilter());
    //glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, spec.asMagFilter());

    // https://stackoverflow.com/questions/42886835/modifying-parameters-of-bindless-resident-textures
    m_handle = glGetTextureSamplerHandleARB(m_frameBuffer->m_spec.attachments[0].textureID, m_samplerId);
    glMakeTextureHandleResidentARB(m_handle);
}

void ShaderMaterialUpdater::prepareArray()
{
}

void ShaderMaterialUpdater::updateTexture()
{
    const auto& assets = Assets::get();
    if (assets.drawUseArrayTexture) {
        updateSingle();
    }
    else {
        updateArray();
    }
}

void ShaderMaterialUpdater::updateSingle()
{
}

void ShaderMaterialUpdater::updateArray()
{
    // TODO KI copy frameuffer
}

void ShaderMaterialUpdater::render(
    const render::RenderContext& ctx)
{
    m_dirty |= m_frameCounter++ > m_frameSkip;

    if (!m_dirty) return;
    if (!m_frameBuffer) return;

    m_dirty = false;
    m_frameCounter = 0;

    if (!m_material) return;

    auto programId = m_material->getProgram(material::ProgramType::shader);
    if (!programId) return;

    m_frameBuffer->bind(ctx);
    m_frameBuffer->clearAll();

    auto& state = ctx.getGLState();

    glBindSampler(UNIT_CHANNEL_0, m_samplerId);

    {
        auto* program = Program::get(programId);
        program->bind();
        program->m_uniforms->u_materialIndex.set(m_material->m_registeredIndex);
    }

    {
        render::TextureQuad::get().draw();
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
    updateTexture();
    glFlush();

    setNeedUpdate(true);
}

GLuint64 ShaderMaterialUpdater::getTexHandle(material::TextureType type) const noexcept
{
    if (type == material::TextureType::diffuse) {
        return m_handle;
        //return m_samplerId;
    }
    return 0;
}
