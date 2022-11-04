#include "ImageTexture.h"

#include <map>
#include <mutex>


namespace {
    const int MIP_MAP_LEVELS = 3;

    std::map<const std::string, std::unique_ptr<ImageTexture>> textures;

    std::mutex textures_lock;
}

ImageTexture* ImageTexture::getTexture(const std::string& path, const TextureSpec& spec)
{
    std::lock_guard<std::mutex> lock(textures_lock);

    const std::string cacheKey = path + "_" + std::to_string(spec.mode);

    auto e = textures.find(cacheKey);
    if (e == textures.end()) {
        textures[cacheKey] = std::make_unique<ImageTexture>(path, spec);
        e = textures.find(cacheKey);
        e->second->load();
    }
    return e->second.get();
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

    if (image->channels == 3) {
        format = GL_RGB;
        internalFormat = GL_RGB8;
    } else if (image->channels == 4) {
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
    } else {
        KI_WARN_SB("IMAGE: unsupported channels " << image->channels);
        m_valid = false;
        image.reset();
        return;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, spec.mode);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, spec.mode);

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureStorage2D(textureID, MIP_MAP_LEVELS, internalFormat, image->width, image->height);
    glTextureSubImage2D(textureID, 0, 0, 0, image->width, image->height, format, GL_UNSIGNED_BYTE, image->data);

    glGenerateTextureMipmap(textureID);

    image.reset();
}

void ImageTexture::load() {
    image = std::make_unique<Image>(name);
    int res = image->load(true);
    if (res) {
        image.reset();
        return;
    }

    if (image->channels != 3 && image->channels != 4) {
        KI_WARN_SB("IMAGE: unsupported channels " << image->channels);
        image.reset();
        return;
    }

    m_valid = true;
}
