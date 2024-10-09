#include "TextureQuad.h"

#include "kigl/kigl.h"

namespace {
    static render::TextureQuad g_instance;
}

namespace render {
    TextureQuad& TextureQuad::get() noexcept
    {
        return g_instance;
    }

    void TextureQuad::prepare()
    {
    }

    void TextureQuad::draw()
    {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void TextureQuad::drawInstanced(int instanceCount)
    {
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
    }
}
