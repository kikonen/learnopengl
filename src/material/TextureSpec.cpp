#include "TextureSpec.h"

namespace material
{
    GLenum asFilter(TextureFilter filter)
    {
        switch (filter) {
        case TextureFilter::nearest:
            return GL_NEAREST;
        case TextureFilter::linear:
            return GL_LINEAR;
        case TextureFilter::linear_mipmap_nearest:
            return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::nearest_mipmap_linear:
            return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::linear_mipmap_linear:
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        return GL_NEAREST;
    }
}

namespace material
{
    GLenum TextureSpec::asWrap() const noexcept
    {
        switch (wrap) {
        case WrapMode::repeat:
            return GL_REPEAT;
        case WrapMode::mirrored_repeat:
            return GL_MIRRORED_REPEAT;
        case WrapMode::clamp_to_edge:
            return GL_CLAMP_TO_EDGE;
        case WrapMode::clamp_to_border:
            return GL_CLAMP_TO_BORDER;
        }
        return GL_REPEAT;
    }

    GLenum TextureSpec::asWrapS() const noexcept
    {
        return asWrap();
    }

    GLenum TextureSpec::asWrapT() const noexcept
    {
        return asWrap();
    }

    GLenum TextureSpec::asMinFilter() const noexcept
    {
        return asFilter(minFilter);
    }

    GLenum TextureSpec::asMagFilter() const noexcept
    {
        return asFilter(magFilter);
    }
}
