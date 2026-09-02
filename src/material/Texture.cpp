#include "Texture.h"

#include <mutex>

#include <fmt/format.h>

#include "shader/Program.h"
#include "shader/Shader.h"

#include "util/Util.h"


namespace {
}

Texture::Texture(
    std::string_view name,
    bool grayScale,
    bool gammaCorrect,
    material::TextureType type,
    const material::TextureSpec& spec)
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
        "<TEX: {}, gammaCorrect={}, wrap={}>",
        m_name,
        m_gammaCorrect,
        util::as_integer(m_spec.wrap));
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

int Texture::resolveMixMapLevels()
{
    return 1 + std::min(
        m_spec.maxMipMapLevels,
        static_cast<uint8_t>(log2(std::max(m_width, m_height))));
}
