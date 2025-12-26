#pragma once

#include <string>

#include "animation/VertexJoint.h"

#include "mesh/VaoMesh.h"
#include "mesh/PrimitiveType.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct Rig;
    struct VertexJoint;
}

namespace mesh {
    //
    // **References**
    // https://www.khronos.org/opengl/wiki/Primitive
    // https://stackoverflow.com/questions/39588937/when-drawing-lines-with-vbo-how-do-i-specify-indices
    //
    class PrimitiveMesh final : public VaoMesh
    {
    public:
        PrimitiveMesh();
        PrimitiveMesh(std::string_view name);
        virtual ~PrimitiveMesh();

        void clear();

        virtual const kigl::GLVertexArray* prepareVAO() override;
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        virtual animation::Rig* getRig() const override
        {
            return m_rig.get();
        }

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

        virtual backend::DrawOptions::Mode getDrawMode() override;

    public:
        PrimitiveType m_type;

        std::vector<animation::VertexJoint> m_vertexJoints;

        std::shared_ptr<animation::Rig> m_rig;
    };
}
