#include "Texture.h"

#include <mutex>

#include <fmt/format.h>

#include "shader/Program.h"
#include "shader/Shader.h"


namespace {
}

Texture::Texture(
    std::string_view name,
    bool grayScale,
    bool gammaCorrect,
    TextureType type,
    const TextureSpec& spec)
    : m_name(name),
    m_grayScale{ grayScale },
    m_gammaCorrect(gammaCorrect),
    m_type{ type },
    m_spec(spec)
{
}

Texture::~Texture()
{
    release();
}

std::string Texture::str() const noexcept
{
    return fmt::format(
        "<TEX: {}, gammaCorrect={}, wrapS={}, wrapT={}>",
        m_name,
        m_gammaCorrect,
        kigl::formatEnum(m_spec.wrapS),
        kigl::formatEnum(m_spec.wrapT));
}

void Texture::release()
{
    if (m_handle) {
        glMakeTextureHandleNonResidentARB(m_handle);
    }
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
    }
}
