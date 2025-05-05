#include "ImageTexture.h"

#include <unordered_map>
#include <mutex>
#include <regex>

#include <iostream>

#include <regex>
#include <fmt/format.h>

#include <ktx.h>

#include <fmt/format.h>

#include "material/Image.h"

#include "util/util.h"
#include "util/Log.h"

#include "kigl/kigl.h"

namespace {
    const std::vector<std::regex> hdrMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

ImageTexture::ImageTexture(
    std::string_view name,
    std::string_view path,
    bool grayScale,
    bool gammaCorrect,
    bool flipY,
    const TextureSpec& spec)
    : Texture{ name, grayScale, gammaCorrect, spec },
    m_flipY{ flipY },
    m_path{ path }
{
}

ImageTexture::~ImageTexture()
{
}

std::string ImageTexture::str() const noexcept
{
    return fmt::format(
        "<IMG: {} {}bit {}ch {}x{} {}{} ({}), [{}, {}], [{}, {}]>",
        m_name,
        m_is16Bbit ? "16" : "8",
        m_channels,
        m_width,
        m_height,
        m_grayScale ? "GRAY " : "",
        kigl::formatEnum(m_internalFormat),
        kigl::formatEnum(m_format),
        kigl::formatEnum(m_spec.wrapS),
        kigl::formatEnum(m_spec.wrapT),
        kigl::formatEnum(m_spec.minFilter),
        kigl::formatEnum(m_spec.magFilter)
        );
}

void ImageTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void ImageTexture::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    if (m_image->m_ktx) {
        prepareKtx();
    }
    else {
        prepareNormal();
    }
}

void ImageTexture::prepareNormal()
{
    m_pixelFormat = GL_UNSIGNED_BYTE;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) format
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_image->m_channels == 1) {
        if (m_image->m_is16Bbit) {
            m_format = GL_RED;
            m_internalFormat = m_grayScale ? GL_RGB16 : GL_R16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = m_grayScale ? GL_RGB8 : GL_R8;
        }
        //m_specialTexture = true;
    }
    else if (m_image->m_channels == 2) {
        m_format = GL_RG;
        m_internalFormat = GL_TEXTURE_SWIZZLE_RGBA;
    }
    else if (m_image->m_channels == 3) {
        if (m_image->m_hdri) {
            // NOTE KI hdri is *linear* (no gamma)
            m_format = GL_RGB;
            m_internalFormat = GL_RGB16F;
            m_pixelFormat = GL_FLOAT;
        }
        else if (m_image->m_is16Bbit) {
            m_format = GL_RGB;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RGB;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
            //m_internalFormat = assets.glPreferredTextureFormatRGB;
        }
    }
    else if (m_image->m_channels == 4) {
        if (m_image->m_is16Bbit) {
            m_format = GL_RGBA;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RGBA;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            //m_internalFormat = assets.glPreferredTextureFormatRGBA;
        }
    }
    else {
        KI_WARN(fmt::format(
            "IMAGE: unsupported channels {}, image={}",
            m_image->m_channels, str()));
        m_valid = false;
        m_image.reset();
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

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

        const uint8_t mipMapLevels = std::min(
            m_spec.mipMapLevels,
            static_cast<uint8_t>(log2(std::max(m_image->m_width, m_image->m_height))));

        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_image->m_width, m_image->m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_image->m_width, m_image->m_height, m_format, m_pixelFormat, m_image->m_data);
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO_OUT(fmt::format(
            "TEX_UPLOAD: {}, compressed={}\n{}",
            m_path,
            compFlag,
            str()));

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }

    //m_texIndex = Texture::nextIndex();

    m_image.reset();
}

void ImageTexture::prepareKtx()
{
    ktxTexture* kTexture;
    KTX_error_code result;
    //ktx_size_t offset;
    //ktx_uint8_t* image;
    //ktx_uint32_t level, layer, faceSlice;
    GLenum target, glerror;

    result = ktxTexture_CreateFromNamedFile(
        m_image->m_path.c_str(),
        KTX_TEXTURE_CREATE_NO_FLAGS,
        &kTexture);

    if (result) {
        KI_ERROR(fmt::format("Failed to open ktx: {}", m_image->m_path));
        m_image.reset();
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    result = ktxTexture_GLUpload(kTexture, &m_textureID, &target, &glerror);
    ktxTexture_Destroy(kTexture);

    if (result) {
        KI_ERROR(fmt::format("Failed to upload ktx: {}", m_image->m_path));
        m_image.reset();
        return;
    }

    GLint compFlag;
    glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
    KI_INFO_OUT(fmt::format(
        "TEX_UPLOAD: path={}, compressed={}\n{}",
        m_image->m_path,
        compFlag,
        str()));

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    m_image.reset();
}

void ImageTexture::load() {
    m_hdri = util::matchAny(hdrMatchers, m_path);

    m_image = std::make_unique<Image>(m_path, m_flipY, m_hdri);
    int res = m_image->load();
    if (res) {
        m_image.reset();
        return;
    }

    m_is16Bbit = m_image->m_is16Bbit;
    m_width = m_image->m_width;
    m_height = m_image->m_height;
    m_channels = m_image->m_channels;
    m_is16Bbit = m_image->m_is16Bbit;

    m_valid = true;
}
