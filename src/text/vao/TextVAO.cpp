#include "TextVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "util/thread.h"

#include "shader/Shader.h"

#include "mesh/vao/TextureEntry.h"
#include "mesh/vao/VBO_impl.h"

namespace text {
    TextVAO::TextVAO(std::string_view name)
        : mesh::TexturedVAO{ name },
        m_atlaxTexVbo{ "vbo_font", ATTR_FONT_ATLAS_TEX, VBO_FONT_ATLAS_BINDING }
    {}

    TextVAO::~TextVAO() = default;

    void TextVAO::prepareVAO()
    {
        TexturedVAO::prepareVAO();

        auto& vao = *m_vao;

        m_atlaxTexVbo.prepareVAO(vao);
    }

    void TextVAO::clear()
    {
        TexturedVAO::clear();

        m_atlaxTexVbo.clear();
    }

    uint32_t TextVAO::reserveAtlasCoords(size_t count)
    {
        ASSERT_RT();

        return m_atlaxTexVbo.reserveVertices(count);
    }

    void TextVAO::updateAtlasCoords(
        uint32_t baseVbo,
        std::span<glm::vec2> atlasCoords)
    {
        ASSERT_RT();

        m_atlaxTexVbo.updateVertices(baseVbo, atlasCoords);
    }

    void TextVAO::updateRT()
    {
        TexturedVAO::updateRT();

        m_atlaxTexVbo.updateVAO(*m_vao);
    }
}
