#pragma once

#include <glm/glm.hpp>

#include "mesh/vao/TextureEntry.h"

#include "mesh/vao/VBO.h"

namespace text {
    class AtlasTextureVBO : public mesh::VBO<glm::vec2, mesh::TextureEntry> {
    public:
        AtlasTextureVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual mesh::TextureEntry convertVertex(
            const glm::vec2& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;
    };
}
