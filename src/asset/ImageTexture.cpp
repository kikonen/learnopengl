#include "ImageTexture.h"

#include <unordered_map>
#include <mutex>

#include "fmt/format.h"

#include "asset/Image.h"

#include "ki/GL.h"

namespace {
    std::unordered_map<std::string, std::shared_future<ImageTexture*>> textures;

    bool preparedTexturesReady = false;
    int preparedTexturesLevel = 0;
    std::vector<const ImageTexture*> preparedTextures;

    std::mutex textures_lock{};

    std::shared_future<ImageTexture*> startLoad(ImageTexture* texture)
    {
        /*
std::future<int> spawn_async_task(){
std::promise<int> p;
std::thread t([p=std::move(p)](){ p.set_value(find_the_answer());});
t.detach();
return f;
}        */
        std::promise<ImageTexture*> promise;
        auto future = promise.get_future().share();

        // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
        // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
        auto th = std::thread{
            [texture, p = std::move(promise)]() mutable {
                try {
                   texture->load();
                   p.set_value(texture);
                }
                catch (const std::exception& ex) {
                    KI_CRITICAL(ex.what());
                    p.set_exception(std::make_exception_ptr(ex));
                } catch (...) {
                    p.set_exception(std::make_exception_ptr(std::current_exception()));
                }
            }
        };
        th.detach();

        return future;
    }
}

std::shared_future<ImageTexture*> ImageTexture::getTexture(
    const std::string& name,
    const std::string& path,
    bool gammaCorrect,
    const TextureSpec& spec)
{
    std::lock_guard<std::mutex> lock(textures_lock);

    const std::string cacheKey = fmt::format(
        "{}_{}_{}-{}_{}_{}_{}",
        path, gammaCorrect,
        spec.wrapS, spec.wrapT,
        spec.minFilter, spec.magFilter, spec.mipMapLevels);

    {
        const auto& e = textures.find(cacheKey);
        if (e != textures.end())
            return e->second;
    }

    auto future = startLoad(new ImageTexture(name, path, gammaCorrect, spec));
    textures[cacheKey] = future;
    return future;
}

const std::pair<int, const std::vector<const ImageTexture*>&> ImageTexture::getPreparedTextures()
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

ImageTexture::ImageTexture(
    const std::string& name,
    const std::string& path,
    bool gammaCorrect,
    const TextureSpec& spec)
    : Texture(name, gammaCorrect, spec),
    m_path(path)
{
}

ImageTexture::~ImageTexture()
{
}

void ImageTexture::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    m_pixelFormat = GL_UNSIGNED_BYTE;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) format
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_image->m_channels == 1) {
        if (m_image->m_is_16_bit) {
            m_format = GL_RED;
            m_internalFormat = GL_R16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = GL_R8;
        }
        //m_specialTexture = true;
    } else if (m_image->m_channels == 2) {
        m_format = GL_RG;
        m_internalFormat = GL_TEXTURE_SWIZZLE_RGBA;
    } else if (m_image->m_channels == 3) {
        m_format = GL_RGB;
        m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
        //m_internalFormat = assets.glPreferredTextureFormatRGB;
    } else if (m_image->m_channels == 4) {
        m_format = GL_RGBA;
        m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        //m_internalFormat = assets.glPreferredTextureFormatRGBA;
    } else {
        KI_WARN(fmt::format("IMAGE: unsupported channels {}", m_image->m_channels));
        m_valid = false;
        m_image.reset();
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    glObjectLabel(GL_TEXTURE, m_textureID, m_name.length(), m_name.c_str());

    if (m_specialTexture) {
        glTextureStorage2D(m_textureID, 1, m_internalFormat, m_image->m_width, m_image->m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_image->m_width, m_image->m_height, m_format, m_pixelFormat, m_image->m_data);

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);
    }
    else {
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

        glTextureStorage2D(m_textureID, m_spec.mipMapLevels, m_internalFormat, m_image->m_width, m_image->m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_image->m_width, m_image->m_height, m_format, m_pixelFormat, m_image->m_data);
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }

    m_texIndex = Texture::nextIndex();

    preparedTexturesReady = false;

    m_image.reset();
}

void ImageTexture::load() {
    m_image = std::make_unique<Image>(m_path);
    int res = m_image->load(true);
    if (res) {
        m_image.reset();
        return;
    }

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) format
    //if (m_image->m_channels != 3 && m_image->m_channels != 4) {
    //    KI_WARN(fmt::format("IMAGE: unsupported channels {}", m_image->m_channels));
    //    m_image.reset();
    //    return;
    //}

    m_valid = true;
}
