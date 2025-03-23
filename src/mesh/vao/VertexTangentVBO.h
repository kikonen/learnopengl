#pragma once

#include <glm/glm.hpp>

#include "mesh/Vertex.h"

#include "TangentEntry.h"

#include "VBO.h"

namespace mesh {
    class VertexTangentVBO : public VBO<Vertex, TangentEntry> {
    public:
        VertexTangentVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual TangentEntry convertVertex(
            const Vertex& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
