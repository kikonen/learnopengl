#pragma once

#include <string>

#include "animation/VertexBone.h"

#include "mesh/Mesh.h"
#include "mesh/PrimitiveType.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct RigContainer;
    struct VertexBone;
}

namespace mesh {
    //
    // **References**
    // https://www.khronos.org/opengl/wiki/Primitive
    // https://stackoverflow.com/questions/39588937/when-drawing-lines-with-vbo-how-do-i-specify-indices
    //
    class PrimitiveMesh final : public Mesh
    {
    public:
        PrimitiveMesh();
        PrimitiveMesh(std::string_view name);
        virtual ~PrimitiveMesh();

        void clear();

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual std::shared_ptr<animation::RigContainer> getRigContainer() override
        {
            return m_rig;
        }

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

    public:
        PrimitiveType m_type;

        std::vector<animation::VertexBone> m_vertexBones;

        std::shared_ptr<animation::RigContainer> m_rig;
    };
}
