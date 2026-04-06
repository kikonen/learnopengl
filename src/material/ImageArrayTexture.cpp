#include "ImageArrayTexture.h"

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

namespace
{
    const std::vector<std::regex> hdrMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

ImageArrayTexture::ImageArrayTexture(
    std::string_view name,
    std::vector<std::string> paths,
    bool shared,
    bool grayScale,
    bool gammaCorrect,
    bool flipY,
    TextureType type,
    const TextureSpec& spec)
    : Texture{ name, grayScale, gammaCorrect, type, spec },
    m_shared{ shared },
    m_flipY{ flipY },
    m_paths{ paths }
{
}

ImageArrayTexture::~ImageArrayTexture()
{
}

std::string ImageArrayTexture::str() const noexcept
{
    return fmt::format(
        "<IMG: {}[{}] {}bit {}ch {}x{} {}{} ({}), [{}, {}], [{}, {}]>",
        m_name,
        m_paths.size(),
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

bool ImageArrayTexture::validateLayers()
{
    if (m_images.empty()) return false;

    const auto* ref = m_images[0].get();
    bool refHdri = util::matchAny(hdrMatchers, ref->m_path);

    for (int i = -1; const auto& img : m_images) {
        i++;
        bool hdri = util::matchAny(hdrMatchers, ref->m_path);

        if (img->m_width != ref->m_width ||
            img->m_height != ref->m_height ||
            img->m_channels != ref->m_channels ||
            img->m_is16Bbit != ref->m_is16Bbit ||
            hdri != refHdri) {
            KI_ERROR(fmt::format(
                "TEX::ARRAY::MISMATCH: layer {} differs from layer 0", i));
            return false;
        }
    }
    return true;
}

void ImageArrayTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void ImageArrayTexture::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    if (m_images[0]->m_ktx) {
        prepareKtx();
    }
    else {
        prepareNormal();
    }
}

void ImageArrayTexture::prepareNormal()
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
            "TEX::ARRAY::PREPARE: unsupported channels {}, image={}",
            m_channels, str()));
        m_valid = false;
        if (!m_shared) {
            m_images.clear();
        }
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE_2D_ARRAY, m_textureID, m_name);

    {
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

        const int layerCount = m_images.size();
        const uint8_t mipMapLevels = std::min(
            m_spec.maxMipMapLevels,
            static_cast<uint8_t>(log2(std::max(m_width, m_height))));

        glTextureStorage3D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height, layerCount);

        for (int layer = 0; layer < layerCount; layer++) {
            glTextureSubImage3D(
                m_textureID,
                0,
                0, 0, layer,
                m_width, m_height,
                1,
                m_format,
                m_pixelFormat,
                m_images[layer]->m_data);
        }

        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO(fmt::format(
            "TEX::ARRAY::UPLOAD::PLAIN: name={}, compressed={}\n{}",
            m_name,
            compFlag,
            str()));

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }

    //m_texIndex = Texture::nextIndex();

    if (!m_shared) {
        m_images.clear();
    }
}

void ImageArrayTexture::prepareKtx()
{
    ktxTexture2* tex2{ nullptr };
    KTX_error_code result;
    //ktx_size_t offset;
    //ktx_uint8_t* image;
    //ktx_uint32_t level, layer, faceSlice;
    GLenum target, glerror;

    const auto* image = m_images[0].get();

    if (!util::fileExists(image->m_path)) {
        KI_ERROR(fmt::format("TEX::ARRAY::UPLOAD::KTX::FILE_NOT_FOUND: path={}", image->m_path));
        if (!m_shared) {
            m_images.clear();
        }
        return;
    }

    if (false) {
        ktxTexture2_CreateFromMemory(
            image->m_data,
            image->m_width,
            KTX_TEXTURE_CREATE_NO_FLAGS, &tex2);
    }

    result = ktxTexture2_CreateFromNamedFile(
        image->m_path.c_str(),
        KTX_TEXTURE_CREATE_NO_FLAGS,
        &tex2);

    if (result != KTX_SUCCESS) {
        KI_ERROR(fmt::format("TEX::ARRAY::UPLOAD::KTX::LOAD: path={}", image->m_path));
        if (!m_shared) {
            m_images.clear();
        }
        return;
    }

    KI_INFO_OUT(fmt::format(
        "TEX::ARRAY::UPLOAD::KTX: path={}, vk_format={}, super_comp_scheme={}, needs_transcoding={}, width={}, height={}, mib_levels={}",
        image->m_path,
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
            KI_ERROR(fmt::format("TEX::ARRAY::UPLOAD::KTX::TRANSCODE: path={}", image->m_path));
            if (!m_shared) {
                m_images.clear();
            }
            return;
        }

        KI_INFO_OUT(fmt::format(
            "TEX::ARRAY::UPLOAD::KTX::TRANSCODE path={}, vk_format={}, compressed={}",
            image->m_path,
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
            "TEX::ARRAY::UPLOAD::KTX: path={}, result={}, GL error=0x{:04X}",
            image->m_path, (int)result, (int)glerror));

        if (!m_shared) {
            m_images.clear();
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
            "TEX::ARRAY::UPLOAD::KTX: path={}, compressed={}, internal_format=0x{:04X}\n{}",
            image->m_path,
            compFlag,
            internalFormat,
            str()));
    }

    m_handle = glGetTextureHandleARB(m_textureID);
    glMakeTextureHandleResidentARB(m_handle);

    if (!m_shared) {
        m_images.clear();
    }
}

void ImageArrayTexture::load()
{
    m_valid = false;

    m_hdri = util::matchAny(hdrMatchers, m_paths[0]);

    m_images.reserve(m_paths.size());
    for (const auto& path : m_paths) {
        auto image = std::make_unique<Image>(path, m_flipY, m_hdri);
        int res = image->load();
        if (res) {
            image.reset();
            return;
        }
        m_images.push_back(std::move(image));
    }

    const auto* image = m_images[0].get();
    m_is16Bbit = image->m_is16Bbit;
    m_width = image->m_width;
    m_height = image->m_height;
    m_channels = image->m_channels;
    m_is16Bbit = image->m_is16Bbit;

    m_valid = validateLayers();
}
