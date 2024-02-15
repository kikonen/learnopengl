#pragma once

#include <string_view>

#include <glm/glm.hpp>

#include "text/size.h"

namespace mesh {
    class ModelVBO;
    class TextureVBO;
}

struct PrepareContext;
class RenderContext;

namespace text
{
    class FontAtlas;

    class TextDraw
    {
    public:
        TextDraw();
        ~TextDraw();

        void prepareRT(
            const PrepareContext& ctx);

        void updateRT();

        void render(
            const RenderContext& ctx,
            text::font_id fontId,
            std::string_view text,
            glm::vec2& pen,
            mesh::ModelVBO& vbo,
            mesh::TextureVBO& atlasVbo);

        void clear();

    private:
        text::FontAtlas* m_lastFont{ nullptr };
    };
}
