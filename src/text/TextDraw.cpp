#include "TextDraw.h"

#include <glm/glm.hpp>

#include <freetype-gl/texture-font.h>

#include "model/Node.h"
#include "model/Snapshot.h"

#include "render/RenderContext.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"

#include "mesh/TextMesh.h"
#include "mesh/Vertex.h"

#include "mesh/vao/VBO_impl.h"

#include "FontRegistry.h"
#include "FontAtlas.h"
#include "FontHandle.h"

namespace
{
    const char* MISSING_CH = "?";
    const char* M_CH = "M";

    //
    // Generate verteces for glyphs into buffer
    //
    void addText(
        mesh::TextMesh* mesh,
        text::FontAtlas* fontAtlas,
        std::string_view text,
        const glm::vec2& pivot,
        text::Align alignHorizontal,
        text::Align alignVertical,
        size_t maxCount)
    {
        if (text.empty()) return;

        if (alignHorizontal == text::Align::none) {
            alignHorizontal = text::Align::left;
        }
        if (alignVertical == text::Align::none) {
            alignVertical = text::Align::top;
        }

        std::vector<size_t> lineOffsets;
        std::vector<size_t> lineLenghts;
        std::vector<float> lineHeights;
        std::vector<const ftgl::texture_glyph_t*> glyphs;
        size_t lineCount = 0;
        float lineHeight = 0.f;

        {
            const glm::vec2 penOrigin = pivot;
            glm::vec2 pen{ 0.f };

            const glm::vec3 normal{ 0.f, 0.f, 1.f };
            const glm::vec3 tangent{ 1.f, 0.f, 0.f };
            const char* prev = { nullptr };

            auto* font = fontAtlas->getFont()->m_font;

            // @see freetype-gl/text-buffer.c
            const auto line_ascender = font->ascender;
            const auto line_descender = font->descender;
            const auto line_height = line_ascender - line_descender;

            //pen.y += line_ascender;

            const ftgl::texture_glyph_t* glyph = texture_font_get_glyph(font, M_CH);
            const float glyphMaxW = glyph->s1 - glyph->s0;
            const float glyphMaxH = glyph->t1 - glyph->t0;

            lineOffsets.push_back(mesh->m_vertices.size());
            lineLenghts.push_back(0);
            lineHeight = line_height;
            lineCount = 1;

            size_t textIndex = 0;
            // https://stackoverflow.com/questions/9438209/for-every-character-in-string
            for (const char& ch : text) {
                if (textIndex >= maxCount) break;

                float line_top = pen.y + line_ascender;

                if (ch == '\n') {
                    pen.x = penOrigin.x;
                    pen.y -= line_height;
                    //pen.y += line_descender;

                    lineOffsets.push_back(mesh->m_vertices.size());
                    lineLenghts.push_back(0);
                    lineCount++;

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

                const GLuint index = (GLuint)mesh->m_vertices.size();

                const glm::vec3 positions[4]{
                    { x0, y0, 0.f },
                    { x0, y1, 0.f },
                    { x1, y1, 0.f },
                    { x1, y0, 0.f },
                };
                const glm::vec2 atlasCoords[4]{
                    { s0, t0 },
                    { s0, t1 },
                    { s1, t1 },
                    { s1, t0 },
                };

                const glm::vec2 materialCoords[4]{
                    { 0, 0 },
                    { 0,                  glyphH / glyphMaxH },
                    { glyphW / glyphMaxW, glyphH / glyphMaxH },
                    { glyphW / glyphMaxW, 0 },
                };

                const mesh::Index32 indeces[2 * 3]{
                    index, index + 1, index + 2,
                    index, index + 2, index + 3,
                };

                for (int i = 0; i < 4; i++) {
                    mesh->m_vertices.emplace_back(positions[i], materialCoords[i], normal, tangent);
                    mesh->m_atlasCoords.emplace_back(atlasCoords[i]);
                }

                for (const auto& v : indeces) {
                    mesh->m_indeces.push_back(v);
                }

                pen.x += glyph->advance_x;
                prev = &ch;

                glyphs.push_back(glyph);
                lineLenghts[lineCount - 1]++;

                textIndex++;
            }

            if (textIndex == 0) {
                // NOTE KI no actual text written
                return;
            }
        }

        // NOTE KI align horizontal
        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto offset = lineOffsets[lineIndex];
            const auto lineLen = lineLenghts[lineIndex];

            float lineW = 0.f;
            for (size_t chIndex = 0; chIndex < lineLen; chIndex++) {
                auto* glyph = glyphs[chIndex];
                lineW += glyph->advance_x;
            }

            float adjustX = 0.f;

            switch (alignHorizontal) {
            case text::Align::left:
                // NOTE KI nothing
                break;
            case text::Align::right:
                adjustX = -lineW;
                break;
            case text::Align::center:
                adjustX = -lineW * 0.5f;
                break;
            }

            for (size_t chIndex = 0; chIndex < lineLen * 4; chIndex++) {
                auto& vertex = mesh->m_vertices[offset + chIndex];
                vertex.pos.x += adjustX;
            }
        }

        // NOTE KI align vertical
        const auto totalHeight = lineCount * lineHeight;

        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto offset = lineOffsets[lineIndex];
            const auto lineLen = lineLenghts[lineIndex];

            float adjustY = 0.f;

            switch (alignVertical) {
            case text::Align::top:
                // NOTE KI nothing
                adjustY = -lineHeight;
                break;
            case text::Align::bottom:
                adjustY += totalHeight - lineHeight;
                break;
            case text::Align::center:
                adjustY += totalHeight * 0.5f - lineHeight;
                break;
            }

            for (size_t chIndex = 0; chIndex < lineLen * 4; chIndex++) {
                auto& vertex = mesh->m_vertices[offset + chIndex];
                vertex.pos.y += adjustY;
            }
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
    }

    void TextDraw::render(
        text::font_id fontId,
        std::string_view text,
        const glm::vec2& pivot,
        text::Align alignHorizontal,
        text::Align alignVertical,
        mesh::TextMesh* mesh)
    {
        auto* font = text::FontRegistry::get().getFont(fontId);
        if (!font) return;

        addText(mesh, font, text, pivot, alignHorizontal, alignVertical, mesh->m_maxSize);

        mesh->m_vertexCount = static_cast<uint32_t>(mesh->m_vertices.size());
        mesh->m_indexCount = static_cast<uint32_t>(mesh->m_indeces.size());
    }

    void TextDraw::clear()
    {
    }
}
