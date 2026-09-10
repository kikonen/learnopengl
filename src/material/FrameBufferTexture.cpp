#include "FrameBufferTexture.h"

#include "shader/Shader.h"

#include "render/FrameBuffer.h"

#include "material/ArrayTexture.h"

FrameBufferTexture::FrameBufferTexture(
    std::string_view name,
    bool grayScale,
    bool gammaCorrect,
    material::TextureType type,
    const material::TextureSpec& spec,
    util::Ref<render::FrameBuffer> fbo,
    int attachmentIndex)
    : Texture(name, grayScale, gammaCorrect, type, spec),
    m_frameBuffer{ fbo },
    m_attachmentIndex{ attachmentIndex }
{ }

FrameBufferTexture::~FrameBufferTexture()
{
    release();
}

void FrameBufferTexture::release()
{
    Texture::release();

    if (m_samplerId) {
        glDeleteSamplers(1, &m_samplerId);
        m_samplerId = 0;
    }
}

void FrameBufferTexture::prepareSingle() 
{
    m_prepared = true;

    glGenSamplers(1, &m_samplerId);

    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
    //glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
    //glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

    // => perhaps should fill with empty data to avoid random garbage?!?

    m_handle = glGetTextureSamplerHandleARB(m_frameBuffer->m_spec.attachments[0].textureID, m_samplerId);
    glMakeTextureHandleResidentARB(m_handle);
    m_boundBindless = true;
}

void FrameBufferTexture::prepareArray(
    ArrayTexture& arr,
    uint32_t layer) 
{
    m_prepared = true;

    // Fetch and sync dynamic dimensions from the target array block
    m_width = arr.getWidth();
    m_height = arr.getHeight();

    // Assign the pure layout integer layer slice offset position index
    m_handle = static_cast<GLuint64>(layer);

    // Clear the newly claimed layer slice to pure transparent black 
    // to guarantee no random VRAM memory garbage artifact leaks show up
    GLuint arrayID = arr.getTextureID();
    GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearTexSubImage(
        arrayID,
        0,
        0, 0, static_cast<GLint>(layer),
        m_width, m_height, 1,
        arr.getFormat(),
        GL_FLOAT,
        clearColor
    );
}

void FrameBufferTexture::updateSingle() 
{
    if (!m_frameBuffer) return;

    // NOTE KI nothing to do using sampler from framebuffer directly
    glBindSampler(UNIT_CHANNEL_0, m_samplerId);
}

void FrameBufferTexture::updateArray(
    ArrayTexture& arr,
    uint32_t layer) 
{
    if (!m_frameBuffer) return;

    // Isolate the hardware source texture attachment ID bound to the FrameBuffer
    GLuint srcTextureID = m_frameBuffer->m_spec.attachments[m_attachmentIndex].textureID;

    // Isolate destination target hardware array pool ID
    GLuint dstTextureID = arr.getTextureID();

    // Determine safe dimensions to copy over without exceeding bounds profiles
    GLsizei copyW = std::min(static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_frameBuffer->m_spec.width));
    GLsizei copyH = std::min(static_cast<GLsizei>(m_height), static_cast<GLsizei>(m_frameBuffer->m_spec.height));

    // Instant Blit-free GPU-to-GPU memory block transfer copy operation.
    // Transmit texture pixel regions natively inside VRAM space across the bus!
    glCopyImageSubData(
        srcTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
        dstTextureID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<GLint>(layer),
        copyW, copyH, 1
    );
}
