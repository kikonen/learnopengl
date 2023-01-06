#include "ImageTexture.h"

#include <map>
#include <mutex>

#include "fmt/format.h"

#include "ki/GL.h"

namespace {
    std::map<const std::string, std::unique_ptr<ImageTexture>> textures;

    bool preparedTexturesReady = false;
    int preparedTexturesLevel = 0;
    std::vector<const ImageTexture*> preparedTextures;

    std::mutex textures_lock;
}

ImageTexture* ImageTexture::getTexture(const std::string& path, const TextureSpec& spec)
{
    std::lock_guard<std::mutex> lock(textures_lock);

    const std::string cacheKey = path + "_" + std::to_string(spec.clamp);

    auto e = textures.find(cacheKey);
    if (e == textures.end()) {
        textures[cacheKey] = std::make_unique<ImageTexture>(path, spec);
        e = textures.find(cacheKey);
        e->second->load();
    }
    return e->second.get();
}

const std::pair<int, const std::vector<const ImageTexture*>&> ImageTexture::getPreparedTextures()
{
    if (!preparedTexturesReady) {
        preparedTexturesReady = true;
        preparedTexturesLevel++;
        preparedTextures.clear();

        for (const auto& [name, texture] : textures) {
            if (texture->m_handle == 0) continue;
            preparedTextures.emplace_back(texture.get());
        }
    }
    return { preparedTexturesLevel, preparedTextures };
}

ImageTexture::ImageTexture(const std::string& path, const TextureSpec& spec)
    : Texture(path, spec)
{
}

ImageTexture::~ImageTexture()
{
}

void ImageTexture::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    if (m_image->m_channels == 3) {
        m_format = GL_RGB;
        m_internalFormat = GL_RGB8;
        //m_internalFormat = assets.glPreferredTextureFormatRGB;
    } else if (m_image->m_channels == 4) {
        m_format = GL_RGBA;
        m_internalFormat = GL_RGBA8;
        //m_internalFormat = assets.glPreferredTextureFormatRGBA;
    } else {
        KI_WARN(fmt::format("IMAGE: unsupported channels {}", m_image->m_channels));
        m_valid = false;
        m_image.reset();
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.clamp);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.clamp);

    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

    glTextureStorage2D(m_textureID, m_spec.mipMapLevels, m_internalFormat, m_image->m_width, m_image->m_height);
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_image->m_width, m_image->m_height, m_format, GL_UNSIGNED_BYTE, m_image->m_data);

    glGenerateTextureMipmap(m_textureID);

    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    m_texIndex = Texture::nextIndex();

    preparedTexturesReady = false;

    m_image.reset();
}

void ImageTexture::load() {
    m_image = std::make_unique<Image>(m_name);
    int res = m_image->load(true);
    if (res) {
        m_image.reset();
        return;
    }

    if (m_image->m_channels != 3 && m_image->m_channels != 4) {
        KI_WARN(fmt::format("IMAGE: unsupported channels {}", m_image->m_channels));
        m_image.reset();
        return;
    }

    m_valid = true;
}
