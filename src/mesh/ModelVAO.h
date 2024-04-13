#pragma once

#include <string>

#include "kigl/GLVertexArray.h"

#include "mesh/VertexPositionVBO.h"
#include "mesh/VertexNormalVBO.h"
#include "mesh/VertexTextureVBO.h"
#include "mesh/VertexIndexEBO.h"

namespace mesh {
    class ModelVBO;

    class ModelVAO {
    public:
        ModelVAO(std::string_view name);
        ~ModelVAO();

        void prepare(std::string_view name);
        void clear();

        void bind();
        void unbind();

        // @return VBO for model mesh
        kigl::GLVertexArray* registerModel(ModelVBO& modelVBO);

        const kigl::GLVertexArray* getVAO() const
        {
            return m_vao.get();
        }

        kigl::GLVertexArray* modifyVAO() const
        {
            return m_vao.get();
        }

        void updateRT();

    private:
        void prepareVAO();

    private:
        bool m_prepared{ false };
        std::string m_name;

        std::unique_ptr<kigl::GLVertexArray> m_vao;

        VertexPositionVBO m_positionVbo;
        VertexNormalVBO m_normalVbo;
        VertexTextureVBO m_textureVbo;
        VertexIndexEBO m_indexEbo;
    };
}
