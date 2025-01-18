#pragma once

#include <string>

#include "kigl/GLVertexArray.h"

#include "VertexVBO.h"
#include "VertexIndexEBO.h"

#include "mesh/Index.h"

namespace mesh {
    class TexturedVAO {
    public:
        TexturedVAO(std::string_view name);
        ~TexturedVAO();

        virtual void prepare();
        virtual void clear();

        void bind();
        void unbind();

        uint32_t reserveVertices(size_t count);
        uint32_t reserveIndeces(size_t count);

        void updateVertices(
            uint32_t baseVbo,
            std::span<Vertex> vertices);

        void updateIndeces(
            uint32_t baseEbo,
            std::span<Index32> indeces);

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

        VertexVBO m_vertexVbo;
        VertexIndexEBO m_indexEbo;

    protected:
        bool m_prepared{ false };

        std::unique_ptr<kigl::GLVertexArray> m_vao;
    };
}
