#include "TextDraw.h"

#include <glm/glm.hpp>

#include <freetype-gl/texture-font.h>

#include "mesh/ModelVBO.h"
#include "mesh/TextureVBO.h"

#include "model/Node.h"
#include "model/Snapshot.h"

//#include "mesh/LodMesh.h"
//#include "mesh/MeshType.h"

#include "render/RenderContext.h"

#include "engine/UpdateContext.h"

#include "FontAtlas.h"
#include "FontHandle.h"

#include "registry/Registry.h"
#include "registry/FontRegistry.h"

#include "mesh/VBO_impl.h"

namespace
{
    const char* MISSING_CH = "?";
    const char* M_CH = "M";

    //
    // Generate verteces for glyphs into buffer
    //
    void addText(
        mesh::ModelVBO& vbo,
        mesh::TextureVBO& atlasVbo,
        text::FontAtlas* fontAtlas,
        std::string_view text,
        glm::vec2& pen)
    {
        const auto penOrigin = pen;
        const glm::vec3 normal{ 0.f, 0.f, 1.f };
        const glm::vec3 tangent{ 1.f, 0.f, 0.f };
        const char* prev = { nullptr };

        const mesh::NormalEntry normals[4]{
            { normal, tangent },
            { normal, tangent },
            { normal, tangent },
            { normal, tangent },
        };

        auto* font = fontAtlas->getFont()->m_font;

        // @see freetype-gl/text-buffer.c
        const auto line_ascender = font->ascender;
        const auto line_descender = font->descender;
        const auto line_height = line_ascender - line_descender;

        //pen.y += line_ascender;

        const ftgl::texture_glyph_t* m_glyph = texture_font_get_glyph(font, M_CH);
        const float glyphMaxW = m_glyph->s1 - m_glyph->s0;
        const float glyphMaxH = m_glyph->t1 - m_glyph->t0;

        // https://stackoverflow.com/questions/9438209/for-every-character-in-string
        for (const char& ch : text) {
            float line_top = pen.y + line_ascender;

            if (ch == '\n') {
                pen.x = penOrigin.x;
                pen.y -= line_height;
                //pen.y += line_descender;
                continue;
            }
            const ftgl::texture_glyph_t* glyph = texture_font_get_glyph(font, &ch);
            if (!glyph) {
                glyph = texture_font_get_glyph(font, MISSING_CH);
            }

            if (!glyph) continue;

            float kerning = 0.0f;
            if (prev)
            {
                kerning = ftgl::texture_glyph_get_kerning(glyph, prev);
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

            const float glyphW = glyph->s1 - glyph->s0;
            const float glyphH = glyph->t1 - glyph->t0;

            const GLuint index = (GLuint)vbo.m_normalEntries.size();

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
            const mesh::TextureEntry textures[4] {
                { {s0, t0} },
                { {s0, t1} },
                { {s1, t1} },
                { {s1, t0} },
            };

            const mesh::TextureEntry material[4]{
                { {0, 0} },
                { {0,                  glyphH / glyphMaxH} },
                { {glyphW / glyphMaxW, glyphH / glyphMaxH} },
                { {glyphW / glyphMaxW, 0} },
            };

            for (const auto& v : positions) {
                vbo.m_positionEntries.push_back(v);
            }
            for (const auto& v : normals) {
                vbo.m_normalEntries.push_back(v);
            }
            for (const auto& v : textures) {
                atlasVbo.addEntry(v);
                //vbo.m_textureEntries.push_back(v);
            }
            for (const auto& v : positions) {
                //vbo.m_textureEntries.push_back(mesh::TextureEntry{ { v.x, v.y } });
            }
            for (const auto& v : material) {
                vbo.m_textureEntries.push_back(v);
            }
            for (const auto& v : indeces) {
                vbo.m_indexEntries.push_back(v);
            }

            pen.x += glyph->advance_x;
            prev = &ch;
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
        const PrepareContext& ctx)
    {
    }

    void TextDraw::updateRT()
    {
        if (m_lastFont) {
            m_lastFont->bindTextures();
        }
    }

    void TextDraw::render(
        const RenderContext& ctx,
        text::font_id fontId,
        std::string_view text,
        glm::vec2& pen,
        mesh::ModelVBO& vbo,
        mesh::TextureVBO& atlasVbo)
    {
        auto* font = FontRegistry::get().getFont(fontId);
        if (!font) return;

        addText(vbo, atlasVbo, font, text, pen);

        // HACK KI need to encode font somehow int drawOptions and/or VBO
        // => can use VBO, sinse are not shared mesh VBOs like in ModelRegistry
        m_lastFont = font;
    }

    void TextDraw::clear()
    {
    }
}
