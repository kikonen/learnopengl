#pragma once

#include <string_view>

#include "asset/Assets.h"

#include "text/size.h"

#include "mesh/ModelVAO.h"
#include "mesh/ModelVBO.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
}

class RenderContext;
class Program;
class Registry;
class Node;

namespace text
{
    class FontAtlas;

    class TextDraw
    {
    public:
        TextDraw();
        ~TextDraw();

        void prepareRT(
            const Assets& assets,
            Registry* registry);

        void draw(
            const RenderContext& ctx,
            text::font_id fontId,
            std::string_view text,
            backend::DrawOptions& drawOptions,
            const Node* node);

        const kigl::GLVertexArray* getVAO() const noexcept
        {
            return m_vao.getVAO();
        }

    private:
        Program* m_program{ nullptr };

        mesh::ModelVAO m_vao;
        mesh::ModelVBO m_vbo;
    };
}
