#include "ImageTexture.h"

#include <unordered_map>
#include <mutex>
#include <regex>

#include <iostream>

#include <regex>
#include <fmt/format.h>

// https://stackoverflow.com/questions/5159353/how-can-i-get-rid-of-the-imp-prefix-in-the-linker-in-vc
#define KHRONOS_STATIC
#include <ktx.h>

#include <fmt/format.h>

#include "material/Image.h"

#include "util/util.h"
#include "util/Log.h"
#include "util/file.h"

#include "kigl/kigl.h"

namespace {
    const std::vector<std::regex> hdrMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

ImageTexture::ImageTexture(
    std::string_view name,
    std::string_view path,
    bool shared,
    bool grayScale,
    bool gammaCorrect,
    bool flipY,
    TextureType type,
    const TextureSpec& spec)
    : Texture{ name, grayScale, gammaCorrect, type, spec },
    m_shared{ shared },
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
    // => need to convert manually to RGB(A) s
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_channels == 1) {
        if (m_is16Bbit) {
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
    else if (m_channels == 2) {
        // NOTE KI in PNG channel 2 is alpha
        m_format = GL_RG;
        m_internalFormat = GL_TEXTURE_SWIZZLE_RGBA;
    }
    else if (m_channels == 3) {
        if (m_hdri) {
            // NOTE KI hdri is *linear* (no gamma)
            m_format = GL_RGB;
            m_internalFormat = GL_RGB16F;
            m_pixelFormat = GL_FLOAT;
        }
        else if (m_is16Bbit) {
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
    else if (m_channels == 4) {
        if (m_is16Bbit) {
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
            "TEX::PREPARE: unsupported channels {}, image={}",
            m_channels, str()));
        m_valid = false;
        if (!m_shared) {
            m_image.reset();
        }
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    if (m_specialTexture) {
        glTextureStorage2D(m_textureID, 1, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_image->m_data);

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
            static_cast<uint8_t>(log2(std::max(m_width, m_height))));

        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_image->m_data);
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO(fmt::format(
            "TEX::UPLOAD: {}, compressed={}\n{}",
            m_path,
            compFlag,
            str()));

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }

    //m_texIndex = Texture::nextIndex();

    if (!m_shared) {
        m_image.reset();
    }
}

void ImageTexture::prepareKtx()
{
    ktxTexture2* tex2{ nullptr };
    KTX_error_code result;
    //ktx_size_t offset;
    //ktx_uint8_t* image;
    //ktx_uint32_t level, layer, faceSlice;
    GLenum target, glerror;

    if (!util::fileExists(m_image->m_path)) {
        KI_ERROR(fmt::format("TEX::UPLOAD::FILE_NOT_FOUND: ktx: {}", m_image->m_path));
        if (!m_shared) {
            m_image.reset();
        }
        return;
    }

    if (false) {
        ktxTexture2_CreateFromMemory(
            m_image->m_data,
            m_image->m_width,
            KTX_TEXTURE_CREATE_NO_FLAGS, &tex2);
    }

    result = ktxTexture2_CreateFromNamedFile(
        m_image->m_path.c_str(),
        KTX_TEXTURE_CREATE_NO_FLAGS,
        &tex2);

    if (result != KTX_SUCCESS) {
        KI_ERROR(fmt::format("TEX::UPLOAD::KTX::LOAD: ktx: {}", m_image->m_path));
        if (!m_shared) {
            m_image.reset();
        }
        return;
    }

    KI_INFO_OUT(fmt::format(
        "TEX::UPLOAD::KTX: path={}, vk_format={}, super_comp_scheme={}, needs_transcoding={}, width={}, height={}, mib_levels={}",
        m_image->m_path,
        (int)tex2->vkFormat,
        (int)tex2->supercompressionScheme,
        (int)ktxTexture2_NeedsTranscoding(tex2),
        (int)tex2->baseWidth,
        (int)tex2->baseHeight,
        (int)tex2->numLevels
        ));

    // Transcode BEFORE uploading
    if (ktxTexture2_NeedsTranscoding(tex2)) {
        // TODO KI KTX_TTF_BC5_RG for normal
        // => will require extra work in shader side
        const auto transcodeFormat = m_type == TextureType::map_normal
            ? KTX_TTF_BC7_RGBA
            : KTX_TTF_BC7_RGBA;

        result = ktxTexture2_TranscodeBasis(tex2, transcodeFormat, 0);
        if (result != KTX_SUCCESS) {
            KI_ERROR(fmt::format("TEX::UPLOAD::KTX::TRANSCODE: ktx: {}", m_image->m_path));
            if (!m_shared) {
                m_image.reset();
            }
            return;
        }

        KI_INFO_OUT(fmt::format(
            "TEX::UPLOAD::KTX::TRANSCODE path={}, vk_format={}, compressed={}",
            m_image->m_path,
            (int)tex2->vkFormat,
            (int)tex2->isCompressed));
    }

    //// https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    //glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    {
        ktxTexture* tex = ktxTexture(tex2);
        result = ktxTexture_GLUpload(tex, &m_textureID, &target, &glerror);
        ktxTexture_Destroy(tex);
    }

    if (result != KTX_SUCCESS) {
        KI_ERROR(fmt::format(
            "TEX::UPLOAD::KTX_UPLOAD: ktx: {}, result={}, GL error=0x{:04X}",
            m_image->m_path, (int)result, (int)glerror));

        if (!m_shared) {
            m_image.reset();
        }
        return;
    }

    {
        kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);
    }

    {
        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);

        GLint internalFormat;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

        KI_INFO(fmt::format(
            "TEX::UPLOAD::KTX: path={}, compressed={}, internal_format=0x{:04X}\n{}",
            m_image->m_path,
            compFlag,
            internalFormat,
            str()));
    }

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    if (!m_shared) {
        m_image.reset();
    }
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
