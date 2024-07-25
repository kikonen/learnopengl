#pragma once

#include <string>

#include "mesh/Mesh.h"

#include "mesh/Index.h"

#include "mesh/Vertex.h"

namespace mesh {
    class TextMesh final : public Mesh
    {
    public:
        TextMesh();
        virtual ~TextMesh();

        void clear();

        virtual const kigl::GLVertexArray* prepareVAO() override;
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

    public:
        std::vector<glm::vec2> m_atlasCoords;

        uint32_t m_maxSize{ 0 };
    };
}
