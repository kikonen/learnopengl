#include "TexturedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "shader/Shader.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "VBO_impl.h"


namespace {
    //constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    //constexpr size_t MAX_VERTEX_BLOCK_COUNT = 10000;

    //constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * MAX_VERTEX_BLOCK_COUNT;
}

namespace mesh {
    TexturedVAO::TexturedVAO(std::string_view name)
        : m_name(name),
        m_positionVbo{ m_name + "_position_vbo", ATTR_POS, VBO_POSITION_BINDING },
        m_normalVbo{ m_name + "_normal_vbo", ATTR_NORMAL, VBO_NORMAL_BINDING },
        m_tangentVbo{ m_name + "_tangent_vbo", ATTR_TANGENT, VBO_TANGENT_BINDING },
        m_textureVbo{ m_name + "_texture_vbo", ATTR_TEX, VBO_TEXTURE_BINDING },
        m_vertexVbo{ m_name + "_vertex_vbo", VBO_VERTEX_BINDING },
        m_indexEbo{ m_name + "_ebo" },
        m_fence{ m_name + "_fence" }
    {
        m_useSeparate = false;
    }

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

        if (m_useSeparate) {
            m_positionVbo.prepareVAO(vao);
            m_normalVbo.prepareVAO(vao);
            m_tangentVbo.prepareVAO(vao);
            m_textureVbo.prepareVAO(vao);
        }
        else {
            m_vertexVbo.prepareVAO(vao);
        }
        m_indexEbo.prepareVAO(vao);
    }

    void TexturedVAO::clear()
    {
        ASSERT_RT();

        if (m_useSeparate) {
            m_positionVbo.clear();
            m_normalVbo.clear();
            m_tangentVbo.clear();
            m_textureVbo.clear();
        }
        else {
            m_vertexVbo.clear();
        }
        m_indexEbo.clear();
    }

    uint32_t TexturedVAO::reserveVertices(size_t count)
    {
        ASSERT_RT();

        uint32_t baseIndex= 0;

        if (m_useSeparate) {
            baseIndex = m_positionVbo.reserveVertices(count);

            m_normalVbo.reserveVertices(count);
            m_tangentVbo.reserveVertices(count);
            m_textureVbo.reserveVertices(count);
        }
        else {
            baseIndex = m_vertexVbo.reserveVertices(count);
        }

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

        if (m_useSeparate) {
            m_positionVbo.updateVertices(baseVbo, vertices);
            m_normalVbo.updateVertices(baseVbo, vertices);
            m_tangentVbo.updateVertices(baseVbo, vertices);
            m_textureVbo.updateVertices(baseVbo, vertices);
        }
        else {
            m_vertexVbo.updateVertices(baseVbo, vertices);
        }
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

        if (m_useSeparate) {
            m_positionVbo.updateVAO(*m_vao);
            m_normalVbo.updateVAO(*m_vao);
            m_tangentVbo.updateVAO(*m_vao);
            m_textureVbo.updateVAO(*m_vao);
        }
        else {
            m_vertexVbo.updateVAO(*m_vao);
        }

        m_indexEbo.updateVAO(*m_vao);
    }
}
