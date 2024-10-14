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

        virtual const kigl::GLVertexArray* setupVAO(
            mesh::TexturedVAO* vao,
            bool shared) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

        virtual AABB calculateAABB(const glm::mat4& transform) const override;

    public:
        AABB m_aabb;
    };
}
