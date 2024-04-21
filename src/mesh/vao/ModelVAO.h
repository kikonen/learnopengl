#pragma once

#include <string>

#include "kigl/GLVertexArray.h"

#include "mesh/ModelMesh.h"

#include "VertexPositionVBO.h"
#include "VertexNormalVBO.h"
#include "VertexTextureVBO.h"
#include "VertexIndexEBO.h"

namespace mesh {
    class ModelVAO {
    public:
        ModelVAO(std::string_view name);
        ~ModelVAO();

        void prepare(std::string_view name);
        void clear();

        void bind();
        void unbind();

        // @return VAO
        kigl::GLVertexArray* registerModel(
            mesh::ModelMesh* mesh);

        const kigl::GLVertexArray* getVAO() const
        {
            return m_vao.get();
        }

        kigl::GLVertexArray* modifyVAO() const
        {
            return m_vao.get();
        }

        void updateRT();

    protected:
        void prepareVAO();

    public:
        VertexPositionVBO m_positionVbo;
        VertexNormalVBO m_normalVbo;
        VertexTextureVBO m_textureVbo;
        VertexIndexEBO m_indexEbo;

    protected:
        bool m_prepared{ false };
        std::string m_name;

        std::unique_ptr<kigl::GLVertexArray> m_vao;
    };
}
