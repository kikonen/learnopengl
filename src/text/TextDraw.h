#pragma once

#include <string_view>

#include "text/size.h"

#include "mesh/ModelVAO.h"
#include "mesh/ModelVBO.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
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
            backend::DrawOptions& drawOptions,
            bool append);

        void clear();

        const kigl::GLVertexArray* getVAO() const noexcept
        {
            return m_vao.getVAO();
        }

    private:
        mesh::ModelVAO m_vao{ "text" };
        mesh::ModelVBO m_vbo;

        text::FontAtlas* m_lastFont{ nullptr };
    };
}
