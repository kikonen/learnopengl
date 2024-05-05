#include "TexturedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "asset/Shader.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "mesh/ModelMesh.h"

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
        kigl::GLState::get().bindVAO(*m_vao);
    }

    void TexturedVAO::unbind()
    {
        kigl::GLState::get().bindVAO(0);
    }

    void TexturedVAO::prepareVAO()
    {
        auto& vao = *m_vao;

        m_positionVbo.prepareVAO(vao);
        m_normalVbo.prepareVAO(vao);
        m_textureVbo.prepareVAO(vao);
        m_indexEbo.prepareVAO(vao);
    }

    void TexturedVAO::clear()
    {
        m_positionVbo.clear();
        m_normalVbo.clear();
        m_textureVbo.clear();
        m_indexEbo.clear();
    }

    const kigl::GLVertexArray* TexturedVAO::registerModel(
        mesh::ModelMesh* mesh)
    {
        ASSERT_RT();

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        assert(!vertices.empty());
        assert(!indeces.empty());

        {
            mesh->m_positionVboOffset = m_positionVbo.addVertices(vertices);
            mesh->m_indexEboOffset = m_indexEbo.addIndeces(indeces);
        }

        m_normalVbo.addVertices(vertices);
        m_textureVbo.addVertices(vertices);

        return m_vao.get();
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
