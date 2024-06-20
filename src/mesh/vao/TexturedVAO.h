#pragma once

#include <string>

#include "kigl/GLVertexArray.h"

#include "mesh/ModelMesh.h"

#include "VertexPositionVBO.h"
#include "VertexNormalVBO.h"
#include "VertexTextureVBO.h"
#include "VertexIndexEBO.h"

namespace mesh {
    class TexturedVAO {
    public:
        TexturedVAO(std::string_view name);
        ~TexturedVAO();

        virtual void prepare();
        virtual void clear();

        void bind();
        void unbind();

        virtual const kigl::GLVertexArray* registerMesh(
            mesh::ModelMesh* mesh);

        void updateMesh(
            uint32_t baseVbo,
            uint32_t baseEbo,
            mesh::ModelMesh* mesh);

        const kigl::GLVertexArray* getVAO() const
        {
            return m_vao.get();
        }

        kigl::GLVertexArray* modifyVAO() const
        {
            return m_vao.get();
        }

        virtual void updateRT();

    protected:
        virtual void prepareVAO();

    public:
        const std::string m_name;

        VertexPositionVBO m_positionVbo;
        VertexNormalVBO m_normalVbo;
        VertexTextureVBO m_textureVbo;
        VertexIndexEBO m_indexEbo;

    protected:
        bool m_prepared{ false };

        std::unique_ptr<kigl::GLVertexArray> m_vao;
    };
}
