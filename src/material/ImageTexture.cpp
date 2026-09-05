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
#include "material/ArrayTexture.h"

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
    material::TextureType type,
    const material::TextureSpec& spec)
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
        "<IMG: {} {}bit {}ch {}x{} {}{} ({}), [{}], [{}, {}]>",
        m_name,
        m_is16Bit ? "16" : "8",
        m_channels,
        m_width,
        m_height,
        m_grayScale ? "GRAY " : "",
        kigl::formatEnum(m_internalFormat),
        kigl::formatEnum(m_format),
        util::as_integer(m_spec.wrap),
        util::as_integer(m_spec.minFilter),
        util::as_integer(m_spec.magFilter)
    );
}

void ImageTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void ImageTexture::prepareSingle()
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid) return;

    if (m_image->m_ktx) {
        prepareKtx();
    }
    else {
        preparePlain();
    }
}

void ImageTexture::prepareArray(
    const util::Ref<ArrayTexture>& arr,
    uint32_t layer)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_valid || !m_image || !m_image->m_data) {
        KI_WARN(fmt::format("TEX::PREPARE_ARRAY: Invalid image data for {}", m_name));
        m_valid = false;
        return;
    }

    // 1. Resolve exact CPU-side raw pixel format mappings (RAM layout)
    // This defines how OpenGL reads the byte array from m_image->m_data
    m_pixelFormat = GL_UNSIGNED_BYTE;

    if (m_channels == 1) {
        m_format = GL_RED;
        if (m_is16Bit) {
            // Upgrade data read stride to 16-bit
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
    }
    else if (m_channels == 2) {
        m_format = GL_RG;
    }
    else if (m_channels == 3) {
        m_format = GL_RGB;
        if (m_hdri) {
            // Read buffer as 32-bit floating point markers
            m_pixelFormat = GL_FLOAT;
        }
        else if (m_is16Bit) {
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
    }
    else if (m_channels == 4) {
        m_format = GL_RGBA;
        if (m_is16Bit) {
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
    }

    // 2. Fetch the global unified hardware ID from the designated ArrayTexture block container
    m_textureID = arr->getTextureID();

    // 3. Directly stream compressed/uncompressed raw pixels straight into the requested array layer slot
    // We override glTextureSubImage2D with glTextureSubImage3D completely!
    glTextureSubImage3D(
        m_textureID,
        0,                                   // Target Mipmap Level 0
        0, 0, static_cast<GLint>(layer),     // xoffset, yoffset, zoffset (The assigned Layer Slot Index!)
        m_width, m_height, 1,                // width, height, layer depth slice count (strictly 1 asset slice)
        m_format,                            // CPU RAM layout configuration mapping (e.g. GL_RGBA, GL_RGB)
        m_pixelFormat,                       // CPU RAM component type size context (GL_UNSIGNED_BYTE / GL_UNSIGNED_SHORT)
        m_image->m_data                      // Hard raw byte memory pointer address context
    );

    // 4. Update the handle signature variable pointer to store the pure tight layer slice integer offset
    // Instead of texture handles, this raw integer is what will be passed into the MaterialSSBO uniform block layout!
    m_handle = static_cast<GLuint64>(layer);

    KI_INFO(fmt::format(
        "TEX::ARRAY::SLOT::UPLOAD: asset={}, streamed into target array textureID={}, assigned layer index={}",
        m_name, m_textureID, layer
    ));

    // Cleanup host RAM allocation memory resources safely if not shared globally
    if (!m_shared) {
        m_image.reset();
    }
}

void ImageTexture::preparePlain()
{
    m_pixelFormat = GL_UNSIGNED_BYTE;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) s
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_channels == 1) {
        if (m_is16Bit) {
            m_format = GL_RED;
            m_internalFormat = GL_R16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = GL_R8;
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
        else if (m_is16Bit) {
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
        if (m_is16Bit) {
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

    {
        if (m_grayScale && m_channels == 1) {
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            glTextureParameteriv(m_textureID, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

        const int mipMapLevels = resolveMixMapLevels();

        glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, mipMapLevels - 1);
        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_image->m_data);
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO(fmt::format(
            "TEX::UPLOAD::PLAIN: path={}, compressed={}\n{}",
            m_path,
            compFlag,
            str()));
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
        KI_ERROR(fmt::format("TEX::UPLOAD::KTX::FILE_NOT_FOUND: path={}", m_image->m_path));
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
        KI_ERROR(fmt::format("TEX::UPLOAD::KTX::LOAD: path={}", m_image->m_path));
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
        const auto transcodeFormat = m_type == material::TextureType::map_normal
            ? KTX_TTF_BC7_RGBA
            : KTX_TTF_BC7_RGBA;

        result = ktxTexture2_TranscodeBasis(tex2, transcodeFormat, 0);
        if (result != KTX_SUCCESS) {
            KI_ERROR(fmt::format("TEX::UPLOAD::KTX::TRANSCODE: path={}", m_image->m_path));
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
            "TEX::UPLOAD::KTX: path={}, result={}, GL error=0x{:04X}",
            m_image->m_path, (int)result, (int)glerror));

        if (!m_shared) {
            m_image.reset();
        }
        return;
    }

    {
        kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());
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

    m_is16Bit = m_image->m_is16Bit;
    m_width = m_image->m_width;
    m_height = m_image->m_height;
    m_channels = m_image->m_channels;
    m_is16Bit = m_image->m_is16Bit;

    m_valid = true;
}
