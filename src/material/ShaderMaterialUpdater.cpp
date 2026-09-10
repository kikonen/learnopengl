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
#include "material/FrameBufferTexture.h"
#include "material/MaterialRegistry.h"
#include "material/TextureRegistry.h"

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
}

void ShaderMaterialUpdater::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_material) return;

    m_size = { 1024, 1024 };

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
    constexpr int ATT_DIFFUSE_INDEX = 0;

    m_texture = util::Ref<FrameBufferTexture>::create(
        fmt::format("fbo_{}", m_name),
        false,
        true,
        material::TextureType::dynamic,
        material::TextureSpec {
            .wrap = material::WrapMode::repeat,
            .minFilter = material::TextureFilter::linear_mipmap_nearest,
            .magFilter = material::TextureFilter::linear,
        },
        m_frameBuffer,
        ATT_DIFFUSE_INDEX
    );

    TextureRegistry::get().registerTexture(m_texture);
}

void ShaderMaterialUpdater::updateTexture()
{
    TextureRegistry::get().updateTexture(m_texture);
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

    {
        auto* program = Program::get(programId);
        program->bind();
        program->m_uniforms->u_materialIndex.set(m_material->m_registeredIndex);
    }

    {
        render::TextureQuad::get().draw();
    }

    // Execute the unified pipeline texture registry stream copy step loop.
    // This dynamically routes execution straight into FrameBufferTexture::updateArray!
    updateTexture();

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
    glFlush();

    setNeedUpdate(true);
}

GLuint64 ShaderMaterialUpdater::getTexHandle(material::TextureType type) const noexcept
{
    if (type == material::TextureType::dynamic) {
        return m_texture->getHandle();
    }
    return 0;
}
