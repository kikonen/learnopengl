#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "animation/VertexJoint.h"

#include "mesh/VaoMesh.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct Rig;
    struct VertexJoint;
}

namespace mesh_set
{
    class ModelImporter;
    class AssimpImporter;
}

namespace loader
{
    class NodeTypeBuilder;
}

namespace mesh
{
    class ModelMesh final : public VaoMesh
    {
        friend class mesh_set::ModelImporter;
        friend class mesh_set::AssimpImporter;
        friend class loader::NodeTypeBuilder;

    public:
        ModelMesh(
            std::string_view name);

        virtual ~ModelMesh();

        virtual const kigl::GLVertexArray* prepareVAO() override;
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh);

        virtual backend::DrawOptions::Mode getDrawMode() override;

        virtual animation::Rig* getRig() const override
        {
            return m_rig.get();
        }

        virtual size_t getDefinedVertexCount() const noexcept override
        {
            return m_vertices.size();
        }

        virtual size_t getDefinedIndexCount() const noexcept override
        {
            return m_indeces.size();
        }

    public:
        std::vector<animation::VertexJoint> m_vertexJoints;

        bool m_smoothNormals{ false };
        bool m_forceNormals{ false };

    private:
        std::shared_ptr<animation::Rig> m_rig;
    };
}
