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

        uint32_t getVertexCount() const noexcept override
        {
            return 0;
        }

        // NOTE KI drawCount == indexCount for non-VAO
        uint32_t getIndexCount() const noexcept override
        {
            return m_drawCount;
        }

        const kigl::GLVertexArray* setupVAO(
            mesh::TexturedVAO* vao,
            bool shared) override;

        backend::DrawOptions::Type getDrawType() const noexcept override
        {
            return backend::DrawOptions::Type::arrays;
        }

        AABB calculateAABB(const glm::mat4& transform) const override;

        void setDrawCount(int drawCount) {
            m_drawCount = drawCount;
        }

    public:
        AABB m_aabb;

    private:
        uint32_t m_drawCount{ 0 };
    };
}
