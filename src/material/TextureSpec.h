#pragma once

#include "kigl/kigl.h"

#include <type_traits>
#include <stdint.h>

namespace material
{
    inline constexpr uint8_t MIP_MAP_LEVELS = 12;

    enum class WrapMode : std::underlying_type_t<std::byte>
    {
        repeat,
        mirrored_repeat,
        clamp_to_edge,
        clamp_to_border
    };

    enum class TextureFilter : std::underlying_type_t<std::byte>
    {
        nearest,
        linear,
        linear_mipmap_nearest,
        nearest_mipmap_linear,
        linear_mipmap_linear,
    };

    struct TextureSpec
    {
        // NOTE KI opengl default is GL_REPEAT
        // => match that
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
        WrapMode wrap = WrapMode::repeat;

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        //
        // NOTE KI GL_NEAREST_MIPMAP_LINEAR is *default*
        TextureFilter minFilter = TextureFilter::linear_mipmap_nearest;
        TextureFilter magFilter = TextureFilter::linear;

        // Max mipmap levels
        uint8_t maxMipMapLevels = MIP_MAP_LEVELS;

        GLenum asWrap() const noexcept;
        GLenum asWrapS() const noexcept;
        GLenum asWrapT() const noexcept;

        GLenum asMinFilter() const noexcept;
        GLenum asMagFilter() const noexcept;

        static TextureSpec noiseSpec();
    };
}
