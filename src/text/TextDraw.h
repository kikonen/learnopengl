#pragma once

#include <string_view>

class RenderContext;

namespace text
{
    class FontAtlas;

    class TextDraw
    {
    public:
        TextDraw();
        ~TextDraw();

        void draw(
            const RenderContext& ctx,
            std::string_view text,
            FontAtlas* font);

    private:
    };
}
