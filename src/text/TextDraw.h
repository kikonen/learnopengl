#pragma once

#include <string_view>

#include <glm/glm.hpp>

#include "text/size.h"
#include "text/align.h"

namespace mesh {
    class TextMesh;
}

namespace render
{
    class RenderContext;
}

struct PrepareContext;

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
            text::font_id fontId,
            std::string_view text,
            const glm::vec2& pivot,
            text::Align alignHorizontal,
            text::Align alignVertical,
            mesh::TextMesh* mesh);

        void clear();
    };
}
