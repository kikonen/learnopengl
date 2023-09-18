#include "ChannelTexture.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "util/glm_format.h"

#include "asset/ImageTexture.h"
#include "asset/Image.h"


namespace {
    std::unordered_map<std::string, std::shared_future<ChannelTexture*>> textures;

    bool preparedTexturesReady = false;
    int preparedTexturesLevel = 0;
    std::vector<const ChannelTexture*> preparedTextures;

    std::mutex textures_lock{};

    std::shared_future<ChannelTexture*> startLoad(ChannelTexture* texture)
    {
        std::promise<ChannelTexture*> promise;
        auto future = promise.get_future().share();

        // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
        // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
        auto th = std::thread {
            [texture, p = std::move(promise)]() mutable {
                try {
                   texture->load();
                   p.set_value(texture);
                }
                catch (const std::exception& ex) {
                    KI_CRITICAL(ex.what());
                    p.set_exception(std::make_exception_ptr(ex));
                }
                catch (...) {
                    p.set_exception(std::make_exception_ptr(std::current_exception()));
                }
            }
        };
        th.detach();

        return future;
    }
}

std::shared_future<ChannelTexture*> ChannelTexture::getTexture(
    const std::string& name,
    const std::vector<ImageTexture*>& sourceTextures,
    const glm::vec4& defaults,
    bool is16Bbit,
    const TextureSpec& spec)
{
    std::lock_guard<std::mutex> lock(textures_lock);

    std::string pathsStr;

    for (auto& tex : sourceTextures) {
        pathsStr.append(";");
        pathsStr.append(tex ? tex->m_path : "-");
    }

    const std::string cacheKey = fmt::format(
        "{}_{}_{}_{}-{}_{}_{}_{}_{}",
        name, pathsStr, defaults, is16Bbit,
        spec.wrapS, spec.wrapT,
        spec.minFilter, spec.magFilter, spec.mipMapLevels);

    {
        const auto& e = textures.find(cacheKey);
        if (e != textures.end())
            return e->second;
    }

    auto future = startLoad(new ChannelTexture(name, sourceTextures, defaults, is16Bbit, spec));
    textures[cacheKey] = future;
    return future;
}

const std::pair<int, const std::vector<const ChannelTexture*>&> ChannelTexture::getPreparedTextures()
{
    if (!preparedTexturesReady) {
        preparedTexturesReady = true;
        preparedTexturesLevel++;
        preparedTextures.clear();

        for (const auto& [name, future] : textures) {
            auto* texture = future.get();
            if (!texture || !texture->m_handle) continue;
            preparedTextures.emplace_back(texture);
        }
    }
    return { preparedTexturesLevel, preparedTextures };
}

ChannelTexture::ChannelTexture(
    const std::string& name,
    const std::vector<ImageTexture*>& sourceTextures,
    const glm::vec4& defaults,
    bool is16Bbit,
    const TextureSpec& spec)
    : Texture(name, false, spec),
    m_sourceTextures{ sourceTextures },
    m_defaults{ defaults },
    m_is16Bbit{ is16Bbit }
{
}

ChannelTexture::~ChannelTexture()
{
    if (m_data) {
        delete[](m_data);
        m_data = nullptr;
    }
}

void ChannelTexture::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    if (m_is16Bbit) {
        m_pixelFormat = GL_UNSIGNED_SHORT;
    }
    else {
        m_pixelFormat = GL_UNSIGNED_BYTE;
    }

    switch (m_sourceTextures.size()) {
    case 4:
        m_format = GL_RGBA;
        m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        break;
    case 3:
        m_format = GL_RGB;
        m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
        break;
    case 2:
        m_format = GL_RG;
        m_internalFormat = GL_RG8;
        break;
    case 1:
        m_format = GL_RED;
        m_internalFormat = GL_R8;
        break;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    glObjectLabel(GL_TEXTURE, m_textureID, m_name.length(), m_name.c_str());

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

    glTextureStorage2D(m_textureID, m_spec.mipMapLevels, m_internalFormat, m_width, m_height);
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data);
    glGenerateTextureMipmap(m_textureID);

    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    m_texIndex = Texture::nextIndex();
}

void ChannelTexture::load()
{
    int w = -1;
    int h = -1;

    int validCount = 0;

    for (auto& sourceTexture : m_sourceTextures) {
        if (!sourceTexture) continue;

        auto& image = sourceTexture->m_image;
        if (!image) continue;

        if (w <= 0) {
            w = image->m_width;
            h = image->m_height;
        }

        bool valid = image->m_width == w && image->m_height == h;
        if (valid)
            validCount++;
    }

    m_valid = validCount > 0;

    if (!m_valid) return;

    const int dstPixelBytes = m_is16Bbit ? 2 : 1;
    const int dstRGBA = m_sourceTextures.size();

    const int bufferSize = w * dstPixelBytes * h * dstRGBA;

    m_data = new unsigned char[bufferSize];
    memset(m_data, 1, bufferSize);

    unsigned char* dstByteData{ nullptr };
    unsigned short* dstShortData{ nullptr };

    if (m_is16Bbit) {
        dstShortData = (unsigned short*)m_data;
    }
    else {
        dstByteData = m_data;
    }

    int dstOffset = -1;

    m_width = w;
    m_height = h;

    for (auto& tex : m_sourceTextures) {
        dstOffset++;
        int defaultValue = m_defaults[dstOffset] * (m_is16Bbit ? 65535 : 255);
        //if (!tex) continue;

        auto* image = tex ? tex->m_image.get() : nullptr;
        //if (!image) continue;

        const int srcPixelBytes = image && image->m_is16Bbit ? 2 : 1;
        const float pixelRatio = dstPixelBytes / (float)srcPixelBytes;

        unsigned char* srcByteData{ nullptr };
        unsigned short* srcShortData{ nullptr };

        if (image) {
            if (image->m_is16Bbit) {
                srcShortData = (unsigned short*)image->m_data;
            }
            else {
                srcByteData = image->m_data;
            }
        }

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                const int srcIndex = y * w + x;

                int value;
                if (srcByteData) {
                    value = srcByteData[srcIndex] * pixelRatio;
                }
                else if (srcShortData) {
                    value = srcShortData[srcIndex] * pixelRatio;
                }
                else {
                    value = defaultValue;
                }

                int dstIndex = y * w * dstRGBA + x * dstRGBA + dstOffset;
                if (m_is16Bbit) {
                    dstShortData[dstIndex] = value;
                }
                else {
                    dstByteData[dstIndex] = value;
                }
            }
        }
    }
}
