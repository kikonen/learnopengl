#include "Texture.h"

#include <mutex>

#include "shader/Program.h"
#include "shader/Shader.h"


namespace {
}

Texture::Texture(
    std::string_view name,
    bool gammaCorrect,
    const TextureSpec& spec)
    : m_name(name),
    m_gammaCorrect(gammaCorrect),
    m_spec(spec)
{
}

Texture::~Texture()
{
    if (m_handle) {
        glMakeImageHandleNonResidentARB(m_handle);
    }
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
    }
}
