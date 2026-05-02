#pragma once

#include <string_view>

#include "asset/AABB.h"

#include "Mesh.h"

namespace mesh {
    // Mesh withouut VAO/VBO
    class NonVaoMesh : public Mesh
    {
    public:
        NonVaoMesh(std::string_view name);
        virtual ~NonVaoMesh();

        const kigl::GLVertexArray* setupVAO(
            mesh::TexturedVAO* vao,
            bool shared) override;

        backend::DrawOptions::Type getDrawType() const noexcept override
        {
            return backend::DrawOptions::Type::arrays;
        }

        AABB calculateAABB(const glm::mat4& transform) const override;

        void setDrawCount(int count) {
            m_indexCount = count;
        }

    public:
        AABB m_aabb;
    };
}
