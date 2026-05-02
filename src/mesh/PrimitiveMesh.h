#pragma once

#include <string>

#include "util/Ref.h"

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

        const kigl::GLVertexArray* prepareVAO() override;
        const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        animation::Rig* getRig() const override
        {
            return m_rig.get();
        }

        backend::DrawOptions::Mode getDrawMode() const noexcept override;

    public:
        PrimitiveType m_type;

        std::vector<animation::VertexJoint> m_vertexJoints;

        util::Ref<animation::Rig> m_rig;
    };
}
