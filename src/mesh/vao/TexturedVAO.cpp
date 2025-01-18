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
        m_vertexVbo{ m_name + "_vertex_vbo", VBO_VERTEX_BINDING },
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

        m_vertexVbo.prepareVAO(vao);
        m_indexEbo.prepareVAO(vao);
    }

    void TexturedVAO::clear()
    {
        ASSERT_RT();

        m_vertexVbo.clear();
        m_indexEbo.clear();
    }

    uint32_t TexturedVAO::reserveVertices(size_t count)
    {
        ASSERT_RT();

        auto baseIndex = m_vertexVbo.reserveVertices(count);

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

        m_vertexVbo.updateVertices(baseVbo, vertices);
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

        m_vertexVbo.updateVAO(*m_vao);

        m_indexEbo.updateVAO(*m_vao);
    }
}
