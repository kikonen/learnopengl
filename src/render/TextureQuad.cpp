#include "TextureQuad.h"

#include "kigl/kigl.h"

namespace {
    static render::TextureQuad s_instance;
}

namespace render {
    TextureQuad& TextureQuad::get() noexcept
    {
        return s_instance;
    }

    void TextureQuad::prepare()
    {
    }

    void TextureQuad::draw()
    {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
