#include "TextDraw.h"

#include <glm/glm.hpp>

#include <freetype-gl/texture-font.h>
//#include <freetype-gl/vertex-buffer.h>

#include "asset/Program.h"
#include "asset/ProgramUniforms.h"
#include "asset/Shader.h"

#include "kigl/GLState.h"

#include "mesh/ModelVBO.h"
#include "mesh/ModelVAO.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "mesh/MeshType.h"

#include "render/RenderContext.h"

#include "engine/UpdateContext.h"

#include "TextMaterial.h"
#include "FontAtlas.h"
#include "FontHandle.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"
#include "registry/FontRegistry.h"

namespace
{
    typedef struct {
        float x, y, z;
        float u, v;
        glm::vec4 color;
    } vertex_t;

    //
    // Generate verteces for glyphs into buffer
    //
    void addText(
        mesh::ModelVBO& vbo,
        text::FontAtlas* font,
        std::string_view text,
        glm::vec2 pen)
    {
        const glm::vec3 normal{ 0.f, 0.f, 0.f };
        const glm::vec3 tangent{ 0.f, 0.f, 0.f };
        char prev = '\0';

        for (const char ch : text) {
            const ftgl::texture_glyph_t* glyph = texture_font_get_glyph(font->getFont()->m_font, &ch);

            float kerning = 0.0f;
            if (prev)
            {
                kerning = ftgl::texture_glyph_get_kerning(glyph, &prev);
            }
            pen.x += kerning;

            // Actual glyph
            const float x0 = (pen.x + glyph->offset_x);
            const float y0 = (float)((int)(pen.y + glyph->offset_y));
            const float x1 = (x0 + glyph->width);
            const float y1 = (float)((int)(y0 - glyph->height));
            const float s0 = glyph->s0;
            const float t0 = glyph->t0;
            const float s1 = glyph->s1;
            const float t1 = glyph->t1;

            const GLuint index = (GLuint)vbo.m_vertexEntries.size();

            const mesh::IndexEntry indeces[2] {
                {index, index + 1, index + 2},
                {index, index + 2, index + 3},
            };
            const mesh::PositionEntry positions[4] {
                { { x0, y0, 0.f } },
                { { x0, y1, 0.f } },
                { { x1, y1, 0.f } },
                { { x1, y0, 0.f } },
            };
            const mesh::VertexEntry verteces[4] {
                { normal, tangent, {s0, t0} },
                { normal, tangent, {s0, t1} },
                { normal, tangent, {s1, t1} },
                { normal, tangent, {s1, t0} },
            };

            for (const auto& v : indeces) {
                vbo.m_indexEntries.push_back(v);
            }
            for (const auto& v : verteces) {
                vbo.m_vertexEntries.push_back(v);
            }
            for (const auto& v : positions) {
                vbo.m_positionEntries.push_back(v);
            }

            pen.x += glyph->advance_x;
            prev = ch;
        }
    }
}

namespace text
{
    TextDraw::TextDraw()
    {}

    TextDraw::~TextDraw()
    {}


    void TextDraw::prepareRT(
        const Assets& assets,
        Registry* registry)
    {
        m_vao.prepare("text");

        m_program = registry->m_programRegistry->getProgram(SHADER_FONT_RENDER);
    }

    void TextDraw::draw(
        const RenderContext& ctx,
        text::font_id fontId,
        std::string_view text,
        backend::DrawOptions& drawOptions,
        const Node* node)
    {
        auto* font = ctx.m_registry->m_fontRegistry->getFont(fontId);
        if (!font) return;

        glm::vec2 pen{ 1.f, 1.f };

        m_vbo.clear();
        addText(m_vbo, font, text, pen);

        m_vao.clear();
        m_vao.registerModel(m_vbo);
        m_vao.updateRT(ctx.toUpdateContext());

        drawOptions.indexCount = m_vbo.m_indexEntries.size() * 3;

        font->bindTextures(ctx.m_state);

        if (false) {
            m_vao.bind(ctx.m_state);
            m_program->bind(ctx.m_state);
            font->bindTextures(ctx.m_state);
            {
                const glm::mat4& modelMatrix = node->getSnapshot().getModelMatrix();
                int materialIndex = node->m_type->m_materialIndex;

                m_program->m_uniforms->u_modelMatrix.set(modelMatrix);
                m_program->m_uniforms->u_materialIndex.set(materialIndex);

                // TODO KI actual render
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(m_vbo.m_indexEntries.size()),
                    GL_UNSIGNED_INT,
                    nullptr);
            }
            font->unbindTextures(ctx.m_state);

            m_vao.unbind(ctx.m_state);
        }
    }
}
