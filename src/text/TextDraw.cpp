#include "TextDraw.h"

#include <glm/glm.hpp>

#include <freetype-gl/texture-font.h>

#include "util/utf8_util.h"

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

        size_t vertexOffset{ 0 };
        size_t glyphCount{ 0 };
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

        std::vector<LineInfo> lineInfos;
        float lineHeight = 0.f;
        size_t lineCount = 0;

        {
            const glm::vec2 penOrigin = pivot;
            glm::vec2 pen{ 0.f };

            const glm::vec3 normal{ 0.f, 0.f, 1.f };
            const glm::vec3 tangent{ 1.f, 0.f, 0.f };
            const char* prev = nullptr;

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

            {
                auto& info = lineInfos.emplace_back();
                info.vertexOffset = mesh->m_vertices.size();
            }
            lineHeight = line_height;

            // https://stackoverflow.com/questions/9438209/for-every-character-in-string
            const size_t textLen = text.length();
            const char* textIter = text.data();
            size_t currLine = 0;
            size_t renderedCount = 0;

            for (size_t i = 0; i < textLen; i += util::utf8_surrogate_len(textIter + i))
            {
                const char* codePoint = textIter + i;

                if (renderedCount >= maxCount) break;

                auto& lineInfo = lineInfos[currLine];

                float line_top = pen.y + line_ascender;

                if (*codePoint == '\n') {
                    pen.x = penOrigin.x;
                    pen.y -= line_height;
                    //pen.y += line_descender;

                    auto& info = lineInfos.emplace_back();
                    info.vertexOffset = mesh->m_vertices.size();
                    currLine++;

                    continue;
                }

                const ftgl::texture_glyph_t* glyph = texture_font_get_glyph(font, codePoint);

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

                lineInfo.glyphCount++;

                pen.x += glyph->advance_x;
                prev = codePoint;

                renderedCount++;
            }

            if (renderedCount == 0) {
                // NOTE KI no actual text written
                return;
            }

            lineCount = currLine + 1;
        }

        float top = -1.f;
        float bottom = 1000000.f;

        // NOTE KI align horizontal
        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto& lineInfo = lineInfos[lineIndex];

            top = std::max(top, lineInfo.top);
            bottom = std::min(bottom, lineInfo.bottom);

            const auto vertexOffset = lineInfo.vertexOffset;
            const auto glyphCount = lineInfo.glyphCount;

            float lineW = lineInfo.right - lineInfo.left;
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

            for (size_t glyphIndex = 0; glyphIndex < lineInfo.glyphCount * 4; glyphIndex++) {
                auto& vertex = mesh->m_vertices[vertexOffset + glyphIndex];
                vertex.pos.x += adjustX;
            }
        }

        // NOTE KI align vertical
        // NOTE KI Y-axis goes UP, not DOWN
        const auto totalHeight = abs(bottom - top);

        for (size_t lineIndex = 0; lineIndex < lineCount; lineIndex++) {
            const auto& lineInfo = lineInfos[lineIndex];

            const auto vertexOffset = lineInfo.vertexOffset;
            const auto glyphCount = lineInfo.glyphCount;

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

            for (size_t glyphIndex = 0; glyphIndex < glyphCount * 4; glyphIndex++) {
                auto& vertex = mesh->m_vertices[vertexOffset + glyphIndex];
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
        auto* fontAtlas = text::FontRegistry::get().getFontAtlas(fontId);
        if (!fontAtlas) {
            fontAtlas = text::FontRegistry::get().getDefaultFontAtlas();
        }
        if (!fontAtlas) return;

        const auto atlasPad = fontAtlas->getPadding();
        const auto atlasSize = fontAtlas->getAtlasSize();

        addText(
            mesh, fontAtlas, text, pivot,
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
