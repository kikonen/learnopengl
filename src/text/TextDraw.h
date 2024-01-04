#pragma once

#include <string_view>

#include "asset/Assets.h"

#include "mesh/ModelVAO.h"
#include "mesh/ModelVBO.h"

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
            std::string_view text,
            Node* node);

    private:
        Program* m_program{ nullptr };

        mesh::ModelVAO m_vao;
        mesh::ModelVBO m_vbo;
    };
}
