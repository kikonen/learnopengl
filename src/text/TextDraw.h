#pragma once

#include <string_view>

#include <glm/glm.hpp>

#include "text/size.h"

namespace kigl {
    class GLState;
}

namespace mesh {
    class ModelVBO;
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

        void updateRT(
            kigl::GLState& state);

        void render(
            const RenderContext& ctx,
            text::font_id fontId,
            std::string_view text,
            glm::vec2& pen,
            mesh::ModelVBO& vbo);

        void clear();

    private:
        text::FontAtlas* m_lastFont{ nullptr };
    };
}
