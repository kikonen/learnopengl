#pragma once

#include <glm/glm.hpp>

#include "mesh/Vertex.h"

#include "NormalEntry.h"

#include "VBO.h"

namespace mesh {
    class VertexNormalVBO : public VBO<Vertex, NormalEntry> {
    public:
        VertexNormalVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual NormalEntry convertVertex(
            const Vertex& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
