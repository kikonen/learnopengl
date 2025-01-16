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

    // NOTE KI Y-axis goes UP, not DOWN
    struct LineInfo {
        float left{ 10000000.f };
        float right{ -1.f };
        float top{ -1.f };
        float bottom{ 10000000.f };
    };

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
        float padX,
        float padY,
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
        //std::vector<const ftgl::texture_glyph_t*> glyphs;
        std::vector<LineInfo> lineInfos;
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

            float glyphMaxW;
            float glyphMaxH;
            {
                const ftgl::texture_glyph_t* glyph = texture_font_get_glyph(font, M_CH);
                glyphMaxW = glyph->s1 - glyph->s0;
                glyphMaxH = glyph->t1 - glyph->t0;
            }

            lineOffsets.push_back(mesh->m_vertices.size());
            lineLenghts.push_back(0);
            lineInfos.emplace_back();
            lineHeight = line_height;
            lineCount = 1;

            size_t textIndex = 0;
            // https://stackoverflow.com/questions/9438209/for-every-character-in-string
            for (const char& ch : text) {
                if (textIndex >= maxCount) break;

                auto& lineInfo = lineInfos[lineCount - 1];

                float line_top = pen.y + line_ascender;

                if (ch == '\n') {
                    pen.x = penOrigin.x;
                    pen.y -= line_height;
                    //pen.y += line_descender;

                    lineOffsets.push_back(mesh->m_vertices.size());
                    lineLenghts.push_back(0);
                    lineInfos.emplace_back();
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
                // NOTE KI offset by padding
                const float x0 = pen.x + static_cast<float>(glyph->offset_x);
                const float y0 = pen.y + static_cast<float>(glyph->offset_y);
                const float x1 = x0 + static_cast<float>(glyph->width);
                const float y1 = y0 - static_cast<float>(glyph->height);
                const float s0 = glyph->s0;
                const float t0 = glyph->t0;
                const float s1 = glyph->s1;
                const float t1 = glyph->t1;

                const float glyphW = s1 - s0;
                const float glyphH = t1 - t0;

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

                lineInfo.left = std::min(lineInfo.left, x0);
                lineInfo.right = x0 + glyph->advance_x;
                // NOTE KI Y-axis goes UP, not DOWN
                lineInfo.top = std::max(lineInfo.top, y0);
                lineInfo.bottom = y0 - line_height;

                //glyphs.push_back(glyph);
                lineLenghts[lineCount - 1]++;

                pen.x += glyph->advance_x;
                prev = &ch;

                textIndex++;
            }

            if (textIndex == 0) {
                // NOTE KI no actual text written
                return;
            }
        }

        float top = -1.f;
        float bottom = 1000000.f;

        // NOTE KI align horizontal
        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto& lineInfo = lineInfos[lineIndex];

            top = std::max(top, lineInfo.top);
            bottom = std::min(bottom, lineInfo.bottom);

            const auto offset = lineOffsets[lineIndex];
            const auto lineLen = lineLenghts[lineIndex];

            float lineW = lineInfo.right - lineInfo.left;
            //for (size_t chIndex = 0; chIndex < lineLen; chIndex++) {
            //    auto* glyph = glyphs[chIndex];
            //    lineW += glyph->advance_x;
            //}

            float adjustX = -padX;

            switch (alignHorizontal) {
            case text::Align::left:
                // NOTE KI nothing
                break;
            case text::Align::right:
                adjustX += -lineW;
                break;
            case text::Align::center:
                adjustX += -lineW * 0.5f;
                break;
            }

            for (size_t chIndex = 0; chIndex < lineLen * 4; chIndex++) {
                auto& vertex = mesh->m_vertices[offset + chIndex];
                vertex.pos.x += adjustX;
            }
        }

        // NOTE KI align vertical
        // NOTE KI Y-axis goes UP, not DOWN
        const auto totalHeight = abs(bottom - top);

        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto& lineInfo = lineInfos[lineIndex];

            const auto offset = lineOffsets[lineIndex];
            const auto lineLen = lineLenghts[lineIndex];

            float adjustY = padY - lineHeight;

            switch (alignVertical) {
            case text::Align::top:
                // NOTE KI nothing
                break;
            case text::Align::bottom:
                adjustY += totalHeight;
                break;
            case text::Align::center:
                adjustY += totalHeight * 0.5f;
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

        const auto atlasPad = font->getPadding();
        const auto atlasSize = font->getAtlasSize();

        addText(
            mesh, font, text, pivot,
            alignHorizontal,
            alignVertical,
            //static_cast<float>(atlasPad / atlasSize.x),
            //static_cast<float>(atlasPad / atlasSize.y),
            static_cast<float>(atlasPad),
            static_cast<float>(atlasPad),
            mesh->m_maxSize);

        mesh->m_vertexCount = static_cast<uint32_t>(mesh->m_vertices.size());
        mesh->m_indexCount = static_cast<uint32_t>(mesh->m_indeces.size());
    }

    void TextDraw::clear()
    {
    }
}
