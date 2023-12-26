#include "TextMaterial.h"

#include <freetype-gl/texture-atlas.h>
#include <freetype-gl/texture-font.h>

#include "asset/Shader.h"

//#include "render/FrameBuffer.h"

void TextMaterial::prepareView(
    const Assets& assets,
    Registry* registry)
{
    auto atlas = ftgl::texture_atlas_new(m_atlasSize.x, m_atlasSize.y, 1);

    //ftgl::texture_font_t* font;
    //const char* filename = assets"fonts/Vera.ttf";
    //const char* cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
    //    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    //    "`abcdefghijklmnopqrstuvwxyz{|}~";

    //atlas = texture_atlas_new(512, 512, 1);
    //font = texture_font_new_from_file(atlas, 72, filename);
    //font->rendermode = RENDER_SIGNED_DISTANCE_FIELD;

    //glfwSetTime(total_time);
    //texture_font_load_glyphs(font, cache);
    //total_time += glfwGetTime();

    //texture_font_delete(font);

    //glGenTextures(1, &atlas->id);
    //glBindTexture(GL_TEXTURE_2D, atlas->id);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
    //    0, GL_RED, GL_UNSIGNED_BYTE, atlas->data);

    //GLuint indices[6] = { 0,1,2, 0,2,3 };
    //vertex_t vertices[4] = { { 0,0,0,  0,1,  1,1,1,1 },
    //                         { 0,1,0,  0,0,  1,1,1,1 },
    //                         { 1,1,0,  1,0,  1,1,1,1 },
    //                         { 1,0,0,  1,1,  1,1,1,1 } };
    //buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
    //vertex_buffer_push_back(buffer, vertices, 4, indices, 6);

    //shader = shader_load("shaders/distance-field.vert",
    //    "shaders/distance-field.frag");
    //mat4_set_identity(&projection);
    //mat4_set_identity(&model);
    //mat4_set_identity(&view);

}

void TextMaterial::updateView(const RenderContext& ctx)
{
    if (!m_dirty) return;

    //updateBuffer(ctx);
    m_dirty = false;
}

void TextMaterial::bindTextures(const RenderContext& ctx)
{
    //m_buffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_TEXT);
}

void TextMaterial::unbindTextures(const RenderContext& ctx)
{
    //m_buffer->unbindTexture(ctx, UNIT_TEXT);
}
