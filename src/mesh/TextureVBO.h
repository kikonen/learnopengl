#pragma once

#include <glm/glm.hpp>

#include "mesh/TextureEntry.h"

#include "VBO.h"

namespace mesh {
    class TextureVBO : public VBO<glm::vec2, TextureEntry> {
    public:
        TextureVBO(
            int attr,
            int binding,
            std::string_view name);

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
