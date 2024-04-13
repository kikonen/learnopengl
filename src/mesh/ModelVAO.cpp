#include "ModelVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "asset/Shader.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "mesh/ModelVBO.h"
#include "mesh/ModelMesh.h"

#include "VBO_impl.h"


namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;
}

namespace mesh {
    ModelVAO::ModelVAO(std::string_view name)
        : m_name(name),
        m_positionVbo{ m_name + "_position_vbo" },
        m_normalVbo{ m_name + "_normal_vbo", ATTR_NORMAL, ATTR_TANGENT, VBO_NORMAL_BINDING },
        m_textureVbo{ m_name + "_texture_vbo", ATTR_TEX, VBO_TEXTURE_BINDING },
        m_indexEbo{ m_name + "_ebo" }
    {}

    ModelVAO::~ModelVAO() = default;

    void ModelVAO::prepare(std::string_view name)
    {
        if (m_prepared) return;
        m_prepared = true;

        {
            m_vao = std::make_unique<kigl::GLVertexArray>();
            m_vao->create(name);
        }

        // NOTE KI VBO & EBO are just empty buffers here

        prepareVAO();
    }

    void ModelVAO::bind()
    {
        kigl::GLState::get().bindVAO(*m_vao);
    }

    void ModelVAO::unbind()
    {
        kigl::GLState::get().bindVAO(0);
    }

    void ModelVAO::prepareVAO()
    {
        auto& vao = *m_vao;

        m_positionVbo.prepareVAO(vao);
        m_normalVbo.prepareVAO(vao);
        m_textureVbo.prepareVAO(vao);
        m_indexEbo.prepareVAO(vao);
    }

    void ModelVAO::clear()
    {
        m_positionVbo.clear();
        m_normalVbo.clear();
        m_textureVbo.clear();
        m_indexEbo.clear();
    }

    kigl::GLVertexArray* ModelVAO::registerModel(ModelVBO& modelVBO)
    {
        ASSERT_RT();

        auto& vertices = modelVBO.m_mesh->m_vertices;
        auto& indeces = modelVBO.m_mesh->m_indeces;

        assert(!vertices.empty());
        assert(!indeces.empty());

        {
            modelVBO.m_positionVboOffset = m_positionVbo.addPositions(
                modelVBO.m_meshPositionOffset,
                vertices);
            modelVBO.m_indexEboOffset = m_indexEbo.addIndeces(indeces);
        }

        m_normalVbo.addVertices(vertices);
        m_textureVbo.addVertices(vertices);

        return m_vao.get();
    }

    void ModelVAO::updateRT()
    {
        ASSERT_RT();

        m_positionVbo.updateVAO(*m_vao);
        m_normalVbo.updateVAO(*m_vao);
        m_textureVbo.updateVAO(*m_vao);
        m_indexEbo.updateVAO(*m_vao);
    }
}
