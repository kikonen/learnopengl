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
    const char* MISSING_CH = "?";

    //
    // Generate verteces for glyphs into buffer
    //
    void addText(
        mesh::ModelVBO& vbo,
        text::FontAtlas* fontAtlas,
        std::string_view text,
        glm::vec2 pen)
    {
        const glm::vec3 normal{ 0.f, 0.f, 0.f };
        const glm::vec3 tangent{ 0.f, 0.f, 0.f };
        const char* prev = { nullptr };

        const mesh::NormalEntry normals[4]{
            { normal, tangent },
            { normal, tangent },
            { normal, tangent },
            { normal, tangent },
        };

        auto* font = fontAtlas->getFont()->m_font;

        // https://stackoverflow.com/questions/9438209/for-every-character-in-string
        for (const char& ch : text) {
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

            for (const auto& v : positions) {
                vbo.m_positionEntries.push_back(v);
            }
            for (const auto& v : normals) {
                vbo.m_normalEntries.push_back(v);
            }
            for (const auto& v : textures) {
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
        m_vao.prepare("text");
    }

    void TextDraw::updateRT(
        kigl::GLState& state)
    {
        m_vao.updateRT();
        if (m_lastFont) {
            m_lastFont->bindTextures(state);
        }
    }

    void TextDraw::render(
        const RenderContext& ctx,
        text::font_id fontId,
        std::string_view text,
        backend::DrawOptions& drawOptions,
        bool append)
    {
        auto* font = ctx.m_registry->m_fontRegistry->getFont(fontId);
        if (!font) return;

        glm::vec2 pen{ 1.f, 1.f };

        m_vbo.clear();
        addText(m_vbo, font, text, pen);

        if (!append) {
            m_vao.clear();
        }

        m_vao.registerModel(m_vbo);

        drawOptions.m_vertexOffset = static_cast<ki::uint>(m_vbo.m_vertexOffset);
        drawOptions.m_indexOffset = static_cast<ki::uint>(m_vbo.m_indexOffset);
        drawOptions.m_indexCount = static_cast<ki::uint>(m_vbo.getIndexCount());

        // HACK KI need to encode font somehow int drawOptions and/or VBO
        // => can use VBO, sinse are not shared mesh VBOs like in ModelRegistry
        m_lastFont = font;
    }

    void TextDraw::clear()
    {
        m_vao.clear();
        m_vbo.clear();
    }
}
