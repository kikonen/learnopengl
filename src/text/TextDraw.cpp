#include "TextDraw.h"

#include <glm/glm.hpp>

#include <freetype-gl/texture-font.h>
//#include <freetype-gl/vertex-buffer.h>

#include "FontAtlas.h"

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
    //void addText(
    //    ftgl::vertex_buffer_t* m_pBuffer,
    //    ftgl::texture_font_t* pFont,
    //    char* text,
    //    glm::vec2 pen,
    //    glm::vec4 fg_color_1,
    //    glm::vec4 fg_color_2)
    //{
    //    for (size_t i = 0; i < strlen(text); ++i)
    //    {
    //        ftgl::texture_glyph_t* glyph = texture_font_get_glyph(pFont, text + i);
    //        float kerning = 0.0f;
    //        if (i > 0)
    //        {
    //            kerning = ftgl::texture_glyph_get_kerning(glyph, text + i - 1);
    //        }
    //        pen.x += kerning;

    //        /* Actual glyph */
    //        float x0 = (pen.x + glyph->offset_x);
    //        float y0 = (float)((int)(pen.y + glyph->offset_y));
    //        float x1 = (x0 + glyph->width);
    //        float y1 = (float)((int)(y0 - glyph->height));
    //        float s0 = glyph->s0;
    //        float t0 = glyph->t0;
    //        float s1 = glyph->s1;
    //        float t1 = glyph->t1;
    //        GLuint index = (GLuint)m_pBuffer->vertices->size;
    //        GLuint indices[] = { index, index + 1, index + 2,
    //                            index, index + 2, index + 3 };
    //        vertex_t vertices[] = {
    //            { (float)((int)x0),y0,0,  s0,t0,  fg_color_1 },
    //            { (float)((int)x0),y1,0,  s0,t1,  fg_color_2 },
    //            { (float)((int)x1),y1,0,  s1,t1,  fg_color_2 },
    //            { (float)((int)x1),y0,0,  s1,t0,  fg_color_1 } };
    //        ftgl::vertex_buffer_push_back_indices(m_pBuffer, indices, 6);
    //        ftgl::vertex_buffer_push_back_vertices(m_pBuffer, vertices, 4);
    //        pen.x += glyph->advance_x;
    //    }
    //}
}

namespace text
{
    TextDraw::TextDraw()
    {}

    TextDraw::~TextDraw()
    {}

    void TextDraw::draw(
        const RenderContext& ctx,
        std::string_view text,
        FontAtlas* font)
    {

    }
}
