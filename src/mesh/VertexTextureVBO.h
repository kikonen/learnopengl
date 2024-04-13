#pragma once

#include <glm/glm.hpp>

#include "mesh/Vertex.h"
#include "mesh/TextureEntry.h"

#include "VBO.h"

namespace mesh {
    class VertexTextureVBO : public VBO<Vertex, TextureEntry> {
    public:
        VertexTextureVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual TextureEntry convertVertex(
            const Vertex& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
