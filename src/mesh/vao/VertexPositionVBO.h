#pragma once

#include <string>

#include <glm/glm.hpp>

#include "asset/AABB.h"

#include "mesh/Vertex.h"

#include "PositionEntry.h"

#include "VBO.h"

namespace mesh {
    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    class VertexPositionVBO : public VBO<Vertex, PositionEntry> {
    public:
        VertexPositionVBO(
            std::string_view name,
            int attr,
            int binding);

        virtual PositionEntry convertVertex(
            const Vertex& vertex) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;

        AABB calculateAABB() const noexcept;

    public:
        glm::vec3 m_positionOffset{ 0.f };
    };
}
