#include "TexturedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "shader/Shader.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "VBO_impl.h"


namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;
}

namespace mesh {
    TexturedVAO::TexturedVAO(std::string_view name)
        : m_name(name),
        m_positionVbo{ m_name + "_position_vbo", ATTR_POS, VBO_POSITION_BINDING },
        m_normalVbo{ m_name + "_normal_vbo", ATTR_NORMAL, ATTR_TANGENT, VBO_NORMAL_BINDING },
        m_textureVbo{ m_name + "_texture_vbo", ATTR_TEX, VBO_TEXTURE_BINDING },
        m_indexEbo{ m_name + "_ebo" }
    {}

    TexturedVAO::~TexturedVAO() = default;

    void TexturedVAO::prepare()
    {
        ASSERT_RT();

        if (m_prepared) return;
        m_prepared = true;

        {
            m_vao = std::make_unique<kigl::GLVertexArray>();
            m_vao->create(m_name);
        }

        // NOTE KI VBO & EBO are just empty buffers here

        prepareVAO();
    }

    void TexturedVAO::bind()
    {
        ASSERT_RT();

        kigl::GLState::get().bindVAO(*m_vao);
    }

    void TexturedVAO::unbind()
    {
        ASSERT_RT();

        kigl::GLState::get().bindVAO(0);
    }

    void TexturedVAO::prepareVAO()
    {
        ASSERT_RT();

        auto& vao = *m_vao;

        m_positionVbo.prepareVAO(vao);
        m_normalVbo.prepareVAO(vao);
        m_textureVbo.prepareVAO(vao);
        m_indexEbo.prepareVAO(vao);
    }

    void TexturedVAO::clear()
    {
        ASSERT_RT();

        m_positionVbo.clear();
        m_normalVbo.clear();
        m_textureVbo.clear();
        m_indexEbo.clear();
    }

    uint32_t TexturedVAO::reserveVertices(size_t count)
    {
        ASSERT_RT();

        auto baseIndex = m_positionVbo.reserveVertices(count);

        m_normalVbo.reserveVertices(count);
        m_textureVbo.reserveVertices(count);

        return baseIndex;
    }

    uint32_t TexturedVAO::reserveIndeces(size_t count)
    {
        ASSERT_RT();

        return m_indexEbo.reserveIndeces(count);
    }

    void TexturedVAO::updateVertices(
        uint32_t baseVbo,
        std::span<Vertex> vertices)
    {
        ASSERT_RT();

        m_positionVbo.updateVertices(baseVbo, vertices);
        m_normalVbo.updateVertices(baseVbo, vertices);
        m_textureVbo.updateVertices(baseVbo, vertices);
    }

    void TexturedVAO::updateIndeces(
        uint32_t baseEbo,
        std::span<Index32> indeces)
    {
        ASSERT_RT();

        m_indexEbo.updateIndeces(baseEbo, indeces);
    }

    void TexturedVAO::updateRT()
    {
        ASSERT_RT();

        m_positionVbo.updateVAO(*m_vao);
        m_normalVbo.updateVAO(*m_vao);
        m_textureVbo.updateVAO(*m_vao);

        m_indexEbo.updateVAO(*m_vao);
    }
}
