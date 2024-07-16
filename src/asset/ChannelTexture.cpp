#include "ChannelTexture.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "util/glm_format.h"
#include "util/Log.h"

#include "asset/ImageTexture.h"
#include "asset/Image.h"
#include "asset/ChannelPart.h"


namespace {
    thread_local std::exception_ptr lastException = nullptr;

    std::unordered_map<std::string, std::shared_future<ChannelTexture*>> textures;

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
                    KI_CRITICAL(fmt::format("CHANNEL_TEX_ERROR: {}", ex.what()));
                    lastException = std::current_exception();
                    p.set_exception(lastException);
                }
                catch (const std::string& ex) {
                    KI_CRITICAL(fmt::format("CHANNEL_TEX_ERROR: {}", ex));
                    lastException = std::current_exception();
                    p.set_exception(lastException);
                }
                catch (const char* ex) {
                    KI_CRITICAL(fmt::format("CHANNEL_TEX_ERROR: {}", ex));
                    lastException = std::current_exception();
                    p.set_exception(lastException);
                }
                catch (...) {
                    KI_CRITICAL(fmt::format("CHANNEL_TEX_ERROR: {}", "UNKNOWN_ERROR"));
                    lastException = std::current_exception();
                    p.set_exception(lastException);
                }
            }
        };
        th.detach();

        return future;
    }
}

std::shared_future<ChannelTexture*> ChannelTexture::getTexture(
    std::string_view name,
    const std::vector<ChannelPart>& parts,
    const std::vector<ImageTexture*>& sourceTextures,
    const glm::vec4& defaults,
    int channelCount,
    bool is16Bbit,
    const TextureSpec& spec)
{
    std::string cacheKey;
    {
        std::string pathsStr;
        for (auto& tex : sourceTextures) {
            pathsStr.append(";");
            pathsStr.append(tex ? tex->m_path : "-");
        }

        cacheKey = fmt::format(
            "{}_{}_{}_{}_{}-{}_{}_{}_{}_{}",
            name, pathsStr, defaults, channelCount, is16Bbit,
            spec.wrapS, spec.wrapT,
            spec.minFilter, spec.magFilter, spec.mipMapLevels);
    }

    std::lock_guard lock(textures_lock);
    {
        const auto& e = textures.find(cacheKey);
        if (e != textures.end())
            return e->second;
    }

    auto future = startLoad(new ChannelTexture(name, parts, sourceTextures, defaults, channelCount, is16Bbit, spec));
    textures.insert({ cacheKey, future });
    return future;
}

ChannelTexture::ChannelTexture(
    std::string_view name,
    const std::vector<ChannelPart>& parts,
    const std::vector<ImageTexture*>& sourceTextures,
    const glm::vec4& defaults,
    int channelCount,
    bool is16Bbit,
    const TextureSpec& spec)
    : Texture(name, false, spec),
    m_parts{ parts },
    m_sourceTextures{ sourceTextures },
    m_defaults{ defaults },
    m_channelCount{ channelCount },
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

void ChannelTexture::prepare()
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

    switch (m_channelCount) {
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

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

    const int mipMapLevels = static_cast<int>(log2(std::max(m_width, m_height)));
    glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data);
    glGenerateTextureMipmap(m_textureID);

    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    //m_texIndex = Texture::nextIndex();
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

    const int bufferSize = w * dstPixelBytes * h * m_channelCount;

    m_data = new unsigned char[bufferSize];
    memset(m_data, 0, bufferSize);

    unsigned char* dstByteData{ nullptr };
    unsigned short* dstShortData{ nullptr };

    if (m_is16Bbit) {
        dstShortData = (unsigned short*)m_data;
    }
    else {
        dstByteData = m_data;
    }

    const int dstPixelMax = m_is16Bbit ? 65535 : 255;

    m_width = w;
    m_height = h;

    bool filled[4] = { false, false, false, false };

    for (int partIndex = 0; partIndex < m_parts.size(); partIndex++) {
        const auto& part = m_parts[partIndex];
        const auto& tex = m_sourceTextures[partIndex];

        auto* image = tex ? tex->m_image.get() : nullptr;
        if (!image) continue;

        if (image->m_width != w || image->m_height != h) {
            KI_WARN(fmt::format(
                "CHANNEL_TEX: part={}, ({}, {}) != ({}, {})",
                partIndex, w, h, image->m_width, image->m_height));
            continue;
        }

        unsigned char* srcByteData{ nullptr };
        unsigned short* srcShortData{ nullptr };

        int srcChannelCount = 0;

        if (image) {
            if (image->m_is16Bbit) {
                srcShortData = (unsigned short*)image->m_data;
            }
            else {
                srcByteData = image->m_data;
            }

            srcChannelCount = image->m_channels;
        }

        //const int srcPixelBytes = image && image->m_is16Bbit ? 2 : 1;
        const int srcPixelMax = image && image->m_is16Bbit ? 65535 : 255;
        const float pixelRatio = dstPixelMax / (float)srcPixelMax;

        for (int srcChannelIndex = 0; srcChannelIndex < 4; srcChannelIndex++) {
            if (srcChannelIndex >= srcChannelCount) continue;

            auto channel = part.m_mapping[srcChannelIndex];

            int dstChannelIndex = ChannelPart::getChannelIndex(channel);
            if (dstChannelIndex == -1) continue;

            if (dstChannelIndex >= m_channelCount) continue;

            int defaultValue = (int)(m_defaults[dstChannelIndex] * (m_is16Bbit ? 65535 : 255));

            filled[dstChannelIndex] = true;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int srcIndex = y * (w * srcChannelCount) + x * srcChannelCount + srcChannelIndex;

                    int value;
                    if (srcByteData) {
                        value = srcByteData[srcIndex];
                        value = (int)(value * pixelRatio);
                    }
                    else if (srcShortData) {
                        value = srcShortData[srcIndex];
                        value = (int)(value * pixelRatio);
                    }
                    else {
                        value = defaultValue;
                    }

                    int dstIndex = y * w * m_channelCount + x * m_channelCount + dstChannelIndex;
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

    // NOTE KI fill missed values with default
    for (int dstChannelIndex = 0; dstChannelIndex < 4; dstChannelIndex++) {
        if (filled[dstChannelIndex]) continue;
        if (dstChannelIndex >= m_channelCount) continue;

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int value = (int)(m_defaults[dstChannelIndex] * (m_is16Bbit ? 65535 : 255));
                int dstIndex = y * w * m_channelCount + x * m_channelCount + dstChannelIndex;
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
