#pragma once

#include <glm/glm.hpp>

#include "mesh/TextureEntry.h"

#include "VBO.h"

namespace mesh {
    class TextureVBO : public VBO<glm::vec2, TextureEntry> {
    public:
        TextureVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual TextureEntry convertVertex(
            const glm::vec2& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
